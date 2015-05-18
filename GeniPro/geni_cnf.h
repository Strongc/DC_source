/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*                        GRUNDFOS ELECTRONICS A/S                          */
/*                           DK-8850 BJERRINGBRO                            */
/*                                 DENMARK                                  */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*                Project:    GENIpro                                       */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               (C) Copyright Grundfos Electronics A/S, 2000               */
/*                                                                          */
/*                            All rights reserved                           */
/*                                                                          */
/*               --------------------------------------------               */
/*                                                                          */
/*               As this is the  property of  GRUNDFOS  it                  */
/*               must not be passed on to any person not aut-               */
/*               horized  by GRUNDFOS or be  copied or other-               */
/*               wise  utilized by anybody without GRUNDFOS'                */
/*               expressed written permission.                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* MODULE NAME      :   geni_cnf.h                                          */
/*                                                                          */
/* FILE NAME        :   geni_cnf.h                                          */
/*                                                                          */
/* FILE DESCRIPTION :  Configuration of the Geni functionality              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _GENI_CNF_H
#define _GENI_CNF_H
#include "typedef.h"
#include "geni_cnf_factory.h"

/*****************************************************************************/
/* Configuration to memory segments		                                     */
/*****************************************************************************/
#define SEGMENT_CHANGE_ALLOWED  FALSE     // If Processor supports #pragma memory

/*****************************************************************************/
/* Configuration for GENI const table pointer addressing                     */
/* V850 use __huge                                                           */
/*****************************************************************************/
#define GENI_ID_PTR_ADDRESSING // __huge

/*****************************************************************************/
/* Configuration to Real Time Operating System ressources                    */
/*                                                                           */
/* Genipro  need following kind of ressources from the operating system:     */
/*                                                                           */
/* 1. Timer-interrupt     // ( 5 msec )                                      */
/* 2. Process( Task )     // must not be delayed more than 30 msec after     */
/*                        // Genipro run signal                              */
/* 3. Signal_set          // Called by Genipro to signal that process        */
/*                        // must be given control                           */
/* 4. Signal_clear        // Called by Genipro to indicate Process end       */
/*****************************************************************************/
// Takes care of including RTOS files if needed
#define GENI_RTOS_TYPE      GENI_EMB_OS  // GENI_EMB_OS         : EmbOs RTOS
                                       // GENI_GF_OS_TIMER0   : Grundfos RTOS using software timer 0
                                       // GENI_GF_OS_USER_IRQ : Grundfos RTOS using UserIrq
                                       // GENI_GF_OS_USER_DEF : Grundfos RTOS using the settings below
                                       // Disable: Don't use RTOS
// defines for using EMB_OS
#define GENI_TASK_STACK_SIZE  1500       // Specify stack size in integers
#define GENI_TASK_PRIORITY    100      // [0;255] | Specify Geni Task priority

/*****************************************************************************/
/* Configuration of event system in Geni_sys:                                */
/*                                                                           */
/* The size of the event queue depends on number of channels and load of     */
/* the system.                                                               */
/* The EXT_EVENT_QUEUE must be enabled if any master channels is             */
/* included                                                                  */
/*****************************************************************************/
#define EVENT_QUEUE_LEN         16                   // only length of 4, 8, 16.. is allowed
#define EXT_EVENT_QUEUE_LEN     16                  // only length of 4, 8, 16.. is allowed

/*****************************************************************************/
/* Default Access control for Class 0 - 13                                   */
/*                                                                           */
/* Possible settings:                                                        */
/*    Disable: No access                                                     */
/*    GET_ACC: enables GET access                                            */
/*    GET_ACC: enables GET access                                            */
/*    SET_ACC: enables SET access                                            */
/*****************************************************************************/
#define ACCESS_CLASS_0      GET_ACC                 // Access setup for Class 0
#define ACCESS_CLASS_1      GET_ACC + SET_ACC       // Access setup for Class 1
#define ACCESS_CLASS_2      GET_ACC                 // Access setup for Class 2
#define ACCESS_CLASS_3                SET_ACC       // Access setup for Class 3
#define ACCESS_CLASS_4      GET_ACC + SET_ACC       // Access setup for Class 4
#define ACCESS_CLASS_5      GET_ACC + SET_ACC       // Access setup for Class 5
#define ACCESS_CLASS_6      GET_ACC + SET_ACC       // Access setup for Class 6
#define ACCESS_CLASS_7      GET_ACC                 // Access setup for Class 7

#define ACCESS_CLASS_8      Disable                 // Access setup for Class 8
#define ACCESS_CLASS_9      GET_ACC                 // Access setup for Class 9
#define ACCESS_CLASS_10     GET_ACC + SET_ACC       // Access setup for Class 10
#define ACCESS_CLASS_11     GET_ACC                 // Access setup for Class 11
#define ACCESS_CLASS_12     GET_ACC + SET_ACC       // Access setup for Class 12
#define ACCESS_CLASS_13     GET_ACC + SET_ACC       // Access setup for Class 13
#define ACCESS_CLASS_14     GET_ACC                 // Access setup for Class 14
#define ACCESS_CLASS_15     GET_ACC + SET_ACC       // Access setup for Class 15
#define ACCESS_CLASS_16     GET_ACC + SET_ACC       // Access setup for Class 16

/*****************************************************************************/
/* Compile Time Options (CTO's) for select of  classes                       */
/*****************************************************************************/
#define CTO_CLASS_8             Disable             // Enable | Disable
#define CTO_CLASS_9             Enable              // Enable | Disable
// Fill out if CTO_CLASS_9 is enabled
#define VIR_CLASS_9             Disable             // Enable | Disable
#define VIR_CLASS_9_SPECIFIER   COM_CHANNEL         // IR_CHANNEL | BUS_CHANNEL | COM_CHANNEL | PLM_CHANNEL
                                                    // | RS232_CHANNEL
#define CTO_CLASS_10            Enable              // Enable | Disable
#define CTO_CLASS_16_BIT        Enable              // Enable | Disable
#define CTO_CLASS_32_BIT        Enable              // Enable | Disable

/*****************************************************************************/
/* Compile Time Options (CTO's) for select of interface ways                 */
/*                                                                           */
/* CTO_BUF_INSERT_CH_INDX :                                                  */
/*               : Insert the channel index in all buffers when adding new   */
/*               : values. Applies only to the classes which uses buffers    */
/* CTO_BUF_OPT   : Enable user defined way of using API buffers for 8 bit cl.*/
/* Buf_opt_ctr   : User defined way of using API buffers for 8 bit classes   */
/* CTO_BUF16_OPT : Enable user defined way of using API buffers for 16 bit   */
/*               : classes. The way of using buffers for 16 bit classes is   */
/*               : always ID, Value                                          */
/*****************************************************************************/
#define CTO_BUF_INSERT_CH_INDX  Enable               // Enable | Disable
#define CTO_BUF_OPT             Enable               // Enable |  Disable
// Only fill out if CTO_BUF_OPT are enabled
#define Buf_opt_ctr             C4_id_da_buf + C5_id_da_buf     // Buffer access - See Common.h

#define CTO_BUF16_OPT           Enable              //  Enable   |  Disable
#define CTO_BUF32_OPT           Enable              //  Enable   |  Disable

/*****************************************************************************/
/* Optimizing and Target Compiler Options                                    */
/*                                                                           */
/* CRC_OPTIMIZE : Selects if the CRC calculation should be optimized for     */
/*                consumption or execution speed.                            */
/* SH_ADDR      : Specify a Target Compiler dependent Short Address modifier */
/*                Leave empty if this can't be handled by the compiler       */
/* INT_STORAGE  : Specify how the target compiler stores integers. This can  */
/*                be with the low order byte in the lowest address (LO_HI)   */
/*                or with the high order byte in the lowest address (HI_LO)  */
/*****************************************************************************/
#define CRC_OPTIMIZE            Speed               // Speed | Memory
#define SH_ADDR                 //__saddr           // Short Address modifier
                                                    // saddr for d78098x
                                                    // __saddr for d70311x
#define INT_STORAGE             LO_HI               // LO_HI | HI_LO

/*****************************************************************************/
/* Customizing GENIpro API                                                   */
/*                                                                           */
/* DF_buf_len    : Length of receive and transmit buffer                     */
/*                 remember extern declaration if function call is used      */
/*****************************************************************************/
#define DF_buf_len          255                     // [70; 255]
#define BUS_DF_buf_len      DF_buf_len              // [70; 255]
#define IR_DF_buf_len       DF_buf_len              // [70; 255]
#define PLM_DF_buf_len      DF_buf_len              // [70; 255]
#define MDM_DF_buf_len      DF_buf_len              // [70; 255]
#define RS232_DF_buf_len    DF_buf_len              // [70; 255]
#define COM_DF_buf_len      255                     // [35; 255]
#ifdef TCS_USB_SER_PORT
#define USB_DF_buf_len      255                     // [35; 255]
#endif
#define ROUTER_DF_buf_len   66                      // [35; 66]
#define OBJECT_DF_buf_len   64                      // [1 ; 64]
/*****************************************************************************/
/* Specification of Bus Unit Tables and Buffers                              */
/*****************************************************************************/

#define   HIGH_MEAS_ID      FACTORY_HIGH_MEAS_ID
#define   HIGH_CMD_ID       FACTORY_HIGH_CMD_ID
#define   HIGH_CONF_ID      FACTORY_HIGH_CONF_ID
#define   HIGH_REF_ID       FACTORY_HIGH_REF_ID
#define   HIGH_TEST_ID      74      // [0; 255] Highest ID in Class 6, Test Data
#define   HIGH_ASCII_ID     16      // [0; 255] Highest ID in Class 7, ASCII Strings
#define   HIGH_OBJECT_ID    10      // [0; 255] Highest ID in Class 10, data objects

#define   COM_INFO_LEN      35      // [0; 64]  Length of common_info_tab
#define   COM_PTR_LEN       0       // [0; 64]  Length of common_ptr_tab

#define   HIGH_MEAS16_ID    FACTORY_HIGH_MEAS16_ID
#define   HIGH_CONF16_ID    FACTORY_HIGH_CONF16_ID
#define   HIGH_REF16_ID     FACTORY_HIGH_REF16_ID

#define   HIGH_MEAS32_ID    FACTORY_HIGH_MEAS32_ID
#define   HIGH_CONF32_ID    FACTORY_HIGH_CONF32_ID
#define   HIGH_REF32_ID     FACTORY_HIGH_REF32_ID

#define   CMD_BUF_LEN       254     // [6; 254],   Length of Command buffer
#define   CONF_BUF_LEN      254     // [0; 254], Length of Conf. Parameter buffer
#define   REF_BUF_LEN       254     // [0; 254], Length of Reference Value buffer
#define   ASCII_BUF_LEN     0       // [0; 254], Length of Ascii Value buffer
#define   ASCII_SIZE        10      // size of non constant Parameters in Class 7

#define   CONF16_BUF_LEN    254     // [0; 254], Length of Conf16 Parameter buffer
#define   REF16_BUF_LEN     254     // [0; 254], Length of Reference16 Value buffer

#define   CONF32_BUF_LEN    254     // [0; 254], Length of Conf32 Parameter buffer
#define   REF32_BUF_LEN     254     // [0; 254], Length of Reference32 Value buffer

/*****************************************************************************/
/*          Userfunctions called on the following events                     */
/*                                                                           */
/* xxx_rec_xxxx_xxx   : Data are received on this channel                    */
/* xxx_pre_xxxx_xxx   : Telegram's are for this channel but NOT processed    */
/* xxx_post_xxxx_xxx  : Telegram has been processed                          */
/*                                                                           */
/*          Remember extern declaration to Application Program               */
/*          Identifiers used.                                                */
/*                                                                           */
/*          if function will not be used - insert EmptyFunc                  */
/*****************************************************************************/
#define Bus_rec_user_fct             EmptyFunc    // legal C statement
#define Bus_pre_user_fct             EmptyFunc    // legal C statement
#define Bus_post_user_fct            GeniBusDataReceived    // legal C statement
#define Ir_rec_user_fct              EmptyFunc    // legal C statement  // the previous IRBlink
#define Ir_pre_user_fct              EmptyFunc    // legal C statement
#define Ir_post_user_fct             EmptyFunc    // legal C statement
#define Plm_rec_user_fct             EmptyFunc    // legal C statement
#define Plm_pre_user_fct             EmptyFunc    // legal C statement
#define Plm_post_user_fct            EmptyFunc    // legal C statement
#define Com_rec_user_fct             EmptyFunc    // legal C statement
#define Com_pre_user_fct             EmptyFunc    // legal C statement
#define Com_post_user_fct            GeniBusDataReceived    // legal C statement
#define RS232_rec_user_fct           EmptyFunc    // legal C statement
#define RS232_pre_user_fct           EmptyFunc    // legal C statement
#define RS232_post_user_fct          EmptyFunc    // legal C statement
#ifdef TCS_USB_SER_PORT
#define Usb_rec_user_fct             EmptyFunc    // legal C statement
#define Usb_pre_user_fct             EmptyFunc    // legal C statement
#define Usb_post_user_fct            GeniBusDataReceived     // legal C statement
#endif
EXTERN void GeniBusDataReceived(void);

/*****************************************************************************/
/*             Userfunctions for class 10 - objects                          */
/*****************************************************************************/
// the defined function should have prototype like this:
// void function_name(UCHAR id, UINT sub_id)
#define GetObject_fct(id, sub_id, source_address)    GeniObjectRequest(  id, sub_id, source_address)           // legal C statement
#define SetObject_fct(id, sub_id, source_address)    GeniObjectDelivered(id, sub_id, source_address)           // legal C statement

EXTERN void GeniObjectRequest(unsigned char id, unsigned short sub_id, unsigned char source_address);
EXTERN void GeniObjectDelivered(unsigned char id, unsigned short sub_id, unsigned char source_address);

/*****************************************************************************/
/*             Userfunctions for class 8 - memorydump                        */
/*****************************************************************************/
// the defined function should have prototype like this:
//void function_name (UCHAR *cl8_header, UCHAR **addr)
#define GetDump_fct(cl8_header, addr)             // legal C statement
//void function_name (UCHAR *cl8_header)
#define SetDump_fct(cl8_header)                   // legal C statement
/*****************************************************************************/
/*        Userfunction for multipoint master error notification              */
/*****************************************************************************/
// the defined function should have prototype like this:
// void function_name(UCHAR nw_index, UCHAR fault_state)
#define ErrNotifyApp(nw_index, fault_state)  set_geni_error(nw_index, fault_state)  // legal C statement

// the defined function should have prototype like this:
EXTERN void set_geni_error(unsigned char nw_index, unsigned char fault_state);

/*****************************************************************************/
/*        Userfunctions for Point to point master - COM master               */
/*                                                                           */
/*****************************************************************************/
// The defined function should have prototype like this:
// void function_name(UCHAR class, UCHAR id, void *value)
#define DeliverDataItem_fct(a, b,c)               // legal C statement

// The defined function should have prototype like this:
// void function_name(void)
#define DeliverDataEnd_fct                        // legal C statement
// The defined function should have prototype like this:
// void function_name(UCHAR code)
#define DataError_fct(a)                          // legal C statement

/*****************************************************************************/
/*             User function for auto poll reply notification                */
/*****************************************************************************/
// The defined function must have a prototype like this:
//void function_name (UCHAR unit, UCHAR group)
#define AutoPollReplyUserFct(unit, group)             // legal C statement

/*****************************************************************************/
/* Specification of Virtual Slave functionality                              */
/* This can only be used in slave mode                                       */
/*****************************************************************************/
#define USE_VIRTUAL_SLAVES          FALSE            // Use virtual slaves
                                                    // TRUE: use it, FALSE: don't use it
#define MAX_VIR_SLAVE_COUNT         3               // max number of virtual slaves

#define VIR_SLAVE_CHANNEL           PLM_CH          // BUS_CH, PLM_CH, RS232_CH

/*****************************************************************************/
/* Compile Time Options (CTO's) for select of channel type:                  */
/*                                                                           */
/* CTO_BUS_TYPE      :          Genibus channel type                         */
/* CTO_IR_TYPE       :          Ir channel type                              */
/* CTO_PLM_TYPE      :          Plm channel type                             */
/* CTO_COM_TYPE      :          COM channel type                             */
/* CTO_RS232_TYPE    :          RS232 channel type                           */
/*                                                                           */
/*  Remark: Only 4 channels can be enabled in one configuration. Select      */
/*          type or Disable for an given channel and then remember to        */
/*          include or remove the channel modules in the makefile            */
/*****************************************************************************/
#ifndef __PC__  // Target
#define CTO_BUS_TYPE            Slave               //  Disable | Master | Slave
#define CTO_RS232_TYPE          Master              //  Disable | Master | Slave
#define CTO_COM_TYPE            Slave               //  Disable | Master | Slave
#define CTO_PLM_TYPE            Disable             //  Disable | Master | Slave
#define CTO_IR_TYPE             Disable             //  Disable | Master | Slave
#ifdef TCS_USB_SER_PORT
  #define CTO_USB_TYPE            Slave     //usb service port      //  Disable | Master | Slave
#endif
#else  // PC TEST TN
#define CTO_BUS_TYPE            Disable     //GeniBus          //  Disable | Master | Slave
#define CTO_RS232_TYPE          Master      //Reguleringsbus   //  Disable | Master | Slave
#define CTO_COM_TYPE            Slave       //Servicebus       //  Disable | Master | Slave
#define CTO_PLM_TYPE            Disable             //  Disable | Master | Slave
#define CTO_IR_TYPE             Disable             //  Disable | Slave
#endif

/*****************************************************************************/
/*                 Special timing for all channels                           */
/*****************************************************************************/
#define GENI_ADD_DELAY               0              // * 5 ms Additional delay in rx and tx channel together
#define GENI_REPLY_DELAY             0              // * 10 ms - delay the reply to allow slow direction turn arounds in radios

/*****************************************************************************/
/*                              ModBus setup                                 */
/*****************************************************************************/
#define CTO_MOD_BUS                 Disable
#define MODBUS_TAB_SIZE             15
#define MODBUS_START_ADDR           0x0001
/*****************************************************************************/
/*       Size and performance specifications for Multi point Master          */
/*                                                                           */
/*  Fill out if CTO_BUS_TYPE == Master  or CTO_PLM_TYPE == Master or         */
/*  CTO_RS232_TYPE == Master                                                 */
/*****************************************************************************/
#define MAS_MAX_NO_GROUPS            8              // 8 possible parameter groups
#define MAS_MAX_REQ_APDU             50             // Max number of ID's in an request APDU
#define MAS_MAX_UNIT_COUNT           20             // Max number of units on the bus
#define MAS_TICK_TIME                20             // *5 ms - Time for master poll cycle
#define MAS_SHEDULE_METOD            TICK           // [TICK | TIME]
#define MAS_EXTRA_ITEM_POINTER_TAB   Enable         // Enable | Disable
#define MAS_UNIT_ERR_HYS             3              // Number of times unit is allowed not to answer.
#define MAS_DIR_REQ_BUF_SIZE         128            // Must be 16, 32, 64 ....
#define MAS_DIR_REPLY_TIMEOUT        40             // 40*5 ms = 200 ms reply time out (from tgm send to tgm recived)
#define MAS_LAST_RESP_FNC            3              // Index on the last response function for direct tgms

/*****************************************************************************/
/*                                                                           */
/* GENIbus channel definition                                                */
/* Fill out if Genibus channel is enabled                                    */
/*                                                                           */
/*****************************************************************************/
#define BUS_UART_NUM                 1              // UART number - Processor specific see manual
#define BUS_BAUDRATE                 9600           // 9600 | 19200 | 38400
#define BUS_MAX_BAUDRATE             38400          // 0 (Configurable baudrate not supported (fixed to 9600)) | 9600 | 19200 | 38400
#define BUS_UART_PRIORITY            NOT_USED       //INT_PRIO_LEV0  // UART priority - Processor specific see manual
#define BUS_DIR_CTR                  Enable         // Enable | Disable
#define BUS_DIR_PIN                  GENI_GPIO16//CT,1           // Direction pin - Processor specific see manual
#define BUS_MAX_RTY                  2              // Max number of retries
#define BUS_IDLE_TYPE                TIMER_IDLE     // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE
// Fill out if BUS_IDLE_TYPE != NO_IDLE


//TCSL -  Mapping changes for supporting TFT display
//After introduction to TFT, old bus Idle Pins (GPIO 46 and 47) are being utilized for 
//TFT Display. 
//(New Bus Idle Pin - GPMODE 6 - GPIO 52 and 53)
#ifndef TFT_16_BIT_LCD
#define BUS_IDLE_PIN                 GENI_GPIO46    // Idle_pin - Processor specific see manual
#else
#define BUS_IDLE_PIN                 GENI_GPIO52    // Idle_pin - Processor specific see manual
#endif
#define BUS_CONNECT_RXD6_TO_INTP0    FALSE          // Only valid for the D7801xx processor
                                                    // TRUE: Makes an internal connection from RxD6 to INTP0
                                                    // FALSE: Makes no connection.
// Fill out if TIMER_IDLE is selected
#define BUS_IDLE_TIMER               SOFT           // Idle_timer - Processor specific see manual
// Fill out if TIMER_IDLE and SOFT timer are selected
#define BUS_IDLE_COUNT               4              // Number of times SOFT idle routine are called
                                                    // before Idle time has elapsed.
#define BUS_USE_CRC16_CHECK          TRUE           // TRUE | FALSE - Use CRC16 instead of SUM8 check

/*************************************************************************/
/*                                                                       */
/* RS232 channel definition                                              */
/*                                                                       */
/*************************************************************************/
#define RS232_UART_NUM               GENI_NOT_USED  // UART number - Processor specific see manual
#define RS232_BAUDRATE               9600           // 9600 | 19200 | 38400
#define RS232_UART_PRIORITY          NOT_USED       // UART priority - Processor specific see manual
#define RS232_MAX_RTY                2              // max number of retries
#define RS232_DIR_CTR                Enable         // Enable | Disable
#define RS232_DIR_PIN                GENI_GPIO16    // Direction pin - Processor specific see manual                                                   // port_no bit_no eg. 03 for D78098x
#define RS232_IDLE_TYPE              NO_IDLE        // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE
// Fill out if HW_IDLE or TIMER_IDLE is selected

//TCSL -  Mapping changes for supporting TFT display
//After introduction to TFT, old RS232 Idle Pins (GPIO 46 and 47) are being utilized for 
//TFT Display. 
//(New RS232 Idle Pin - GPMODE 6 - GPIO 52 and 53)
#ifndef TFT_16_BIT_LCD
	#define RS232_IDLE_PIN               GENI_GPIO46    // Idle_pin - Processor specific see manual
#else
	#define RS232_IDLE_PIN               GENI_GPIO52    // Idle_pin - Processor specific see manual
#endif
#define RS232_CONNECT_RXD6_TO_INTP0  FALSE          // Only valid for the D7801xx processor
                                                    // TRUE: Makes an internal connection from RxD6 to INTP0
                                                    // FALSE: Makes no connection.
// Fill out if TIMER_IDLE is selected
#define RS232_IDLE_TIMER             SOFT           // Idle_timer - Processor specific see manual
// Fill out if TIMER_IDLE and SOFT timer are selected
#define RS232_IDLE_COUNT             4              // Number of times SOFT idle routine are called
                                                    // before Idle time has elapsed.
#define RS232_USE_CRC16_CHECK        TRUE           // TRUE | FALSE - Use CRC16 instead of SUM8 check

/*****************************************************************************/
/*                                                                           */
/* GENIcom channel definition                                                */
/* Fill out if GENIcom channel is enabled                                    */
/*                                                                           */
/*****************************************************************************/
// Fill out if COM_UART is selected
#define COM_UART_NUM                 2              // UART number - Processor specific see manual
#define COM_USE_CON_REQ              TRUE           // TRUE | FALSE - use connection request on com channel
#define COM_BAUDRATE                 9600           // 9600 | 19200 | 38400
#define COM_UART_PRIORITY            NOT_USED       // UART priority - Processor specific see manual
#define COM_MAX_RTY                  2              // max number of retries
#define COM_DIR_CTR                  Enable         // Enable | Disable
#define COM_DIR_PIN                  GENI_GPIO17    // Direction pin - Processor specific see manual
#ifndef __PC__
#define COM_IDLE_TYPE                TIMER_IDLE     // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE
#else
#define COM_IDLE_TYPE                GENI_IRQ_IDLE  // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE
#endif
//TCSL -  Mapping changes for supporting TFT display
//After introduction to TFT, old COM Idle Pins (GPIO 46 and 47) are being utilized for 
//TFT Display. 
//(New COM Idle Pin - GPMODE 6 - GPIO 52 and 53)
// Fill out if HW_IDLE or TIMER_IDLE is selected
#ifndef TFT_16_BIT_LCD
#define COM_IDLE_PIN                 GENI_GPIO47    // Idle_pin - Processor specific see manual
#else
#define COM_IDLE_PIN                 GENI_GPIO53    // Idle_pin - Processor specific see manual
#endif
#define COM_CONNECT_RXD6_TO_INTP0    FALSE          // Only valid for the D7801xx processor
                                                    // TRUE: Makes an internal connection from RxD6 to INTP0
                                                    // FALSE: Makes no connection.
// Fill out if TIMER_IDLE is selected
#define COM_IDLE_TIMER               SOFT           // Idle_timer - Processor specific see manual
// Fill out if TIMER_IDLE and SOFT timer are selected
#define COM_IDLE_COUNT               4              // Number of times SOFT idle routine are called
                                                    // before Idle time has elapsed.
#define COM_USE_CRC16_CHECK          TRUE          // TRUE | FALSE - specify to use CRC16 instead of SUM8 check
#ifdef TCS_USB_SER_PORT
/*************************************************************************/
/*                                                                       			*/
/* USB service channel Defination                                         */
/* Fill out if USBChannel is enabled                             */
/*                                                                       */
/*************************************************************************/
#define USB_NUM                      1              // USB controller - Processor specific see manual
#define USB_USE_CON_REQ              TRUE           // TS don;t know:?
													//TRUE | FALSE - use connection request on USB channel
#define USB_BAUDRATE                 9600           //Baud rate configuration for USB-CDC class driver

#define USB_MAX_RTY                  3              // max number of retries
#define USB_IDLE_TYPE                TIMER_IDLE     // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE

// Fill out if HW_IDLE or TIMER_IDLE is selected
#define USB_IDLE_PIN                 GENI_NOT_USED  // Idle_pin - Not used as it is used for USB poll only.

#define USB_CONNECT_RXD6_TO_INTP0    FALSE          // TCS don't know:?
													//Only valid for the D7801xx processor
                                                    // TRUE: Makes an internal connection from RxD6 to INTP0
                                                    // FALSE: Makes no connection.
// Fill out if TIMER_IDLE is selected
#define USB_IDLE_TIMER               SOFT           // Idle_timer - Processor specific see manual
// Fill out if TIMER_IDLE and SOFT timer are selected
#define USB_IDLE_COUNT               4              // Number of times SOFT idle routine are called
                                                    // before Idle time has elapsed.
#define USB_USE_CRC16_CHECK          TRUE           // TCS don't know:?
													// TRUE | FALSE - specify to use CRC16 instead of SUM8 check
#endif

/*************************************************************************/
/*                                                                       */
/* Power line channel definition                                         */
/* Fill out if Power line channel is enabled                             */
/*                                                                       */
/*************************************************************************/
#define PLM_UART_NUM                 0              // UART number - Processor specific see manual
// When using a baudrate of 4800 the PLM_MDM_ENHANCED_CTR must be enabled and the application must setup the modem.
#define PLM_BAUDRATE                 2400           // 1200 | 2400 | 4800
#define PLM_UART_PRIORITY            LOW_PRIO       // UART priority - Processor specific see manual
#define PLM_MAX_RTY                  2              // max number of retries
#define PLM_DIR_CTR                  Enable         // Enable | Disable
#define PLM_DIR_PIN                  3,2            // Direction pin - Processor specific see manual
#define PLM_IDLE_TYPE                TIMER_IDLE     // HW_IDLE | TIMER_IDLE  | GENI_IRQ_IDLE | NO_IDLE
// Fill out if HW_IDLE or TIMER_IDLE is selected
#define PLM_IDLE_PIN                 INTP2          // Idle_pin - Processor specific see manual
#define PLM_CONNECT_RXD6_TO_INTP0    False          // Only valid for the D7801xx processor
                                                    // TRUE: Makes an internal connection from RxD6 to INTP0
                                                    // FALSE: Makes no connection.
// Fill out if TIMER_IDLE is selected
#define PLM_IDLE_TIMER               SOFT           // Idle_timer - Processor specific see manual
// Fill out if TIMER_IDLE and SOFT timer are selected
#define PLM_IDLE_COUNT               16             // Number of times SOFT idle routine are called
                                                    // before Idle time has elapsed.
#define PLM_TX_IDLE_COUNT            5              // Number of times SOFT idle routine are called - only used with V850 Ik1
                                                    //
#define PLM_USE_CRC16_CHECK          FALSE          // TRUE | FALSE - specify to use CRC16 instead of SUM8 check

#define PLM_MDM_ENHANCED_CTR         Enable         // Use enhanced features of powerline modem
                                                    // Enable | Disable
// Fill out if PLM_MDM_ENHANCED_CTR is enabled
#define PLM_MDM_SYNC_CLK_PIN         INTP1          // Interrupt input pin for Modem Sync. clock - Processor specific see manual
// Fill out if PLM_MDM_ENHANCED_CTR is enabled
#define PLM_MDM_REG_DATA_PIN         3,3            // Output pin to enable access to modem register - Processor specific see manual


/***************************************************************************/
/*                                                                         */
/* GENIlink channel definition                                             */
/* Fill out if GENIlink channel is enabled                                 */
/*                                                                         */
/***************************************************************************/
#define IR_bank                       Disable       // Disable | [0; 3]  configuration only for K0
#define IR_IRQ_PRIORITY              INT_PRIO_LEV5  // IRQ priority - Only valid for D70332x - must be as high as possible
#define IR_MAX_RTY                   2              // Max number of retries

// Timer used to clamp IR carrier and also as receiver
#define IR_CLAMP_RECEIVER             TP0           //Processor specific, see manual
#define IR_CLAMP_LEVEL                HIGH_LEVEL    // [LOW_LEVEL | HIGH_LEVEL] - Clamp carrier to low or high level
#define IR_CARRIER_TYPE               1             // 0 square wave generator generates carrier
                                                    // 1 PVM timer is used to generate carrier
// Only used if PWM timer is selected for IR_CARRIER_TYPE above and the processor must be the D70332x
#define IR_PWM_WIDTH                  8             // valid values a positive whole value
                                                    // the PWM width in micro seconds can be calculated from the following formular:
                                                    // 8 * IR_PWM_WIDTH / F_CLK_HZ = PWM width in microseconds
                                                    // the F_CLK_HZ is the internal clock frequency in Hz

#define IR_CARRIER_TIMER              TP3           // Carrier generator timer - Processor specific see manual
#define IR_USE_CRC16_CHECK            TRUE          // TRUE | FALSE - Use CRC16 instead of SUM8 check

#endif    //_GENI_CNF_H

