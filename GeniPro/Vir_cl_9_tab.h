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
/* MODULE NAME      :   vir_cl_9_tab.h                                      */
/*                                                                          */
/* FILE NAME        :   vir_cl_9_tab.h                                      */
/*                                                                          */
/* FILE DESCRIPTION :  Interface and config file for class 9 tables         */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/* I N C L U D E S                                                          */
/*                                                                          */
/****************************************************************************/
#ifndef _VIR_CL_9_TAB_H
#define _VIR_CL_9_TAB_H

/*****************************************************************************/
/* Default Access control for Class 0 - 13                                   */
/*                                                                           */
/* Possible settings:                                                        */
/*    Disable: No access                                                     */
/*    GET_ACC: enables GET access                                            */
/*    SET_ACC: enables SET access                                            */
/*****************************************************************************/
#define ACCESS_VIR_CLASS_0      GET_ACC                 // Access setup for Class 0
#define ACCESS_VIR_CLASS_1      GET_ACC + SET_ACC       // Access setup for Class 1
#define ACCESS_VIR_CLASS_2      GET_ACC                 // Access setup for Class 2
#define ACCESS_VIR_CLASS_3                SET_ACC       // Access setup for Class 3
#define ACCESS_VIR_CLASS_4      GET_ACC + SET_ACC       // Access setup for Class 4
#define ACCESS_VIR_CLASS_5      GET_ACC + SET_ACC       // Access setup for Class 5
#define ACCESS_VIR_CLASS_6      Disable                 // Access setup for Class 6
#define ACCESS_VIR_CLASS_7      GET_ACC + SET_ACC       // Access setup for Class 7

#define ACCESS_VIR_CLASS_8      Disable                 // Access setup for Class 8
#define ACCESS_VIR_CLASS_10     Disable                 // Access setup for Class 10
#define ACCESS_VIR_CLASS_11     GET_ACC                 // Access setup for Class 11
#define ACCESS_VIR_CLASS_12     GET_ACC + SET_ACC       // Access setup for Class 12
#define ACCESS_VIR_CLASS_13     GET_ACC + SET_ACC       // Access setup for Class 13

#define ACCESS_VIR_CLASS_14     GET_ACC                 // Access setup for Class 14
#define ACCESS_VIR_CLASS_15     GET_ACC + SET_ACC       // Access setup for Class 15
#define ACCESS_VIR_CLASS_16     GET_ACC + SET_ACC       // Access setup for Class 16

/*****************************************************************************/
/* Options                                                                   */
/*****************************************************************************/
#define CTO_VIR_CLASS_8        Disable              // Enable | Disable
#define CTO_VIR_CLASS_10       Disable              // Enable | Disable
#define CTO_VIR_CLASS_16_BIT   Enable              // Enable | Disable
#define CTO_VIR_CLASS_32_BIT   Enable              // Enable | Disable

/*****************************************************************************/
/* BUFFER LENGTH                                                             */
/*****************************************************************************/

#define   VIR_CMD_BUF_LEN           6               // [6; 254],   Length of Command buffer
#define   VIR_CONF_BUF_LEN          6              // [0,6; 254], Length of Conf. Parameter buffer
#define   VIR_REF_BUF_LEN           6               // [0,6; 254], Length of Reference Value buffer
#define   VIR_ASCII_BUF_LEN         24              // [0,6; 254], Length of ascii buffer
#define   VIR_OBJECT_DF_buf_len     56              // [1 ; 58],  Length of object buffer
#define   VIR_CONF16_BUF_LEN        0               // [0; 254], Length of Conf16 Parameter buffer
#define   VIR_REF16_BUF_LEN         0               // [0; 254], Length of Reference16 Value buffer
#define   VIR_CONF32_BUF_LEN        0               // [0; 254], Length of Conf32 Parameter buffer
#define   VIR_REF32_BUF_LEN         0               // [0; 254], Length of Reference32 Value buffer
#define   VIR_COM_INFO_LEN          17              // [0; 64]  Length of common_info_tab
#define   VIR_COM_PTR_LEN           10              // [0; 64]  Length of common_ptr_tab
/*****************************************************************************/
/* USER FUNCTIONS                                                            */
/*****************************************************************************/
#define vir_cl_9_pre_fct                            // legal C statement
#define vir_cl_9_post_fct                           // legal C statement

 // the defined function should have prototype like this:
// void function_name(UCHAR id, UINT sub_id)
#define VirGetObject_fct(id, sub_id)       // legal C statement
#define VirSetObject_fct(id, sub_id)       // legal C statement

// the defined function should have prototype like this:
//void function_name (UCHAR *cl8_header, UCHAR **addr)
#define VirGetDump_fct(cl8_header, addr)

//void function_name (UCHAR *cl8_header)
#define VirSetDump_fct(cl8_header)

/*****************************************************************************/
/* End of user specifications. Don't change anything below this line!        */
/*****************************************************************************/
extern const PROTAB vir_pre_tab[17];

#if (VIR_COM_INFO_LEN == 0)
  extern const INFO_DATA  vir_common_info_tab[1];
#else
  extern const INFO_DATA  vir_common_info_tab[VIR_COM_INFO_LEN];
#endif

#if (VIR_COM_PTR_LEN == 0)
  extern const INFO_DATA_PTR vir_common_ptr_tab[1];
#else
  extern const INFO_DATA_PTR vir_common_ptr_tab[VIR_COM_PTR_LEN];
#endif

#endif    //_VIR_CL_9_H

