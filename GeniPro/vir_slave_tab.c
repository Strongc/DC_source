/*****************************************************************************/
/*****************************************************************************/
/*******                                                               *******/
/**                                                                         **/
/**               FILE FOR UNIT TABLES TO THE VIRTUAL SLAVES                **/
/**                                                                         **/
/**                                                                         **/
/**                                                                         **/
/**               The following tables are declared:                        **/
/**                                                                         **/
/**               slave_meas_tab                                            **/
/**               slave_meas_info_tab                                       **/
/**               slave_cmd_info_tab                                        **/
/**               slave_conf_tab                                            **/
/**               slave_conf_info_tab                                       **/
/**               slave_ref_tab                                             **/
/**               slave_ref_info_tab                                        **/
/**               slave_test_tab                                            **/
/**               slave_ascii_tab                                           **/
/**                                                                         **/
/*****************************************************************************/
#include "geni_cnf.h"
#include "common.h"
#include "geni_sys.h"
#include "profiles.h"
#include "vir_slave_tab.h"

/*****************************************************************************/
/* pre buffers                                                               */
/*****************************************************************************/
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_BUF)
#endif

#if ( VIR_SLAVE_CONF_BUF_LEN != 0 )
  BUFFER slave_conf_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF_BUF_LEN+2];          // configuration buffer
#endif

#if ( VIR_SLAVE_CMD_BUF_LEN != 0 )
  BUFFER slave_cmd_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CMD_BUF_LEN+2];            // command buffer
#endif

#if ( VIR_SLAVE_REF_BUF_LEN != 0 )
  BUFFER slave_ref_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF_BUF_LEN+2];            // reference buffer
#endif

#if ( VIR_SLAVE_ASCII_BUF_LEN != 0 )
  BUFFER slave_ascii_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_ASCII_BUF_LEN+2];        // ascii buffer
#endif

#if ( VIR_SLAVE_CONF16_BUF_LEN != 0 )
  BUFFER16 slave_conf16_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF16_BUF_LEN+2];    // configuration buffer
#endif

#if ( VIR_SLAVE_REF16_BUF_LEN != 0 )
  BUFFER16 slave_ref16_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF16_BUF_LEN+2];      // reference buffer
#endif

#if ( VIR_SLAVE_CONF32_BUF_LEN != 0 )
  BUFFER32 slave_conf32_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_CONF32_BUF_LEN+2];    // configuration buffer
#endif

#if ( VIR_SLAVE_REF32_BUF_LEN != 0 )
  BUFFER32 slave_ref32_buf[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_REF32_BUF_LEN+2];      // reference buffer
#endif

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif

/****************************************************************************/
/* Class 0 table                                                            */
/****************************************************************************/

// 0 - 19   Accessibility
static const UCHAR df_buf_len        = DF_buf_len;                    // lenght of receive/transmit buf
#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=dataseg(GENI_RAM)
#endif
extern UCHAR unit_bus_mode;

#if (SEGMENT_CHANGE_ALLOWED == TRUE)
  #pragma memory=default
#endif


#if ((CTO_COM_TYPE == Master) && !((CTO_RS232_TYPE != Disable) || (CTO_BUS_TYPE != Disable) || (CTO_PLM_TYPE != Disable) || (CTO_IR_TYPE != Disable)))
static const UINT operation_acc_ctr = 0;
static const UINT operation_acc_ctr_high = 0;
#else
extern UINT operation_acc_ctr;
extern UINT operation_acc_ctr_high;
#endif

static const UCHAR cto_class_8       = CTO_CLASS_8;
static const UCHAR cto_class_9       = CTO_CLASS_9;
static const UCHAR cto_class_10      = CTO_CLASS_10;
static const UCHAR cto_class_16_bit  = CTO_CLASS_16_BIT;
// 20 - 29  Receive and Transmit buffers
static const UCHAR bus_df_buf_len    = BUS_DF_buf_len;
static const UCHAR ir_df_buf_len     = IR_DF_buf_len;
static const UCHAR plm_df_buf_len    = PLM_DF_buf_len;
static const UCHAR rs232_df_buf_len  = RS232_DF_buf_len;
static const UCHAR com_df_buf_len    = COM_DF_buf_len;
// 30 - 49  Buffers
static const UCHAR router_df_buf_len = ROUTER_DF_buf_len;
static const UCHAR object_df_buf_len = OBJECT_DF_buf_len;
static const UCHAR cto_buf_opt       = CTO_BUF_OPT;
#if ( CTO_BUF_OPT == Enable )
extern const UINT buf_opt_ctr;
#else
static const UINT buf_opt_ctr        = FALSE;
#endif
static const UCHAR cmd_buf_len       = VIR_SLAVE_CMD_BUF_LEN;
static const UCHAR conf_buf_len      = VIR_SLAVE_CONF_BUF_LEN;
static const UCHAR ref_buf_len       = VIR_SLAVE_REF_BUF_LEN;
static const UCHAR cto_buf16_opt     = CTO_BUF16_OPT;
static const UCHAR conf16_buf_len    = VIR_SLAVE_CONF16_BUF_LEN;
static const UCHAR ref16_buf_len     = VIR_SLAVE_REF16_BUF_LEN;
static const UCHAR cto_buf32_opt     = CTO_BUF32_OPT;
static const UCHAR conf32_buf_len    = VIR_SLAVE_CONF32_BUF_LEN;
static const UCHAR ref32_buf_len     = VIR_SLAVE_REF32_BUF_LEN;
// 50 - 59 Geni Master
#if (defined MAS_CH)
extern UINT all_master_errors;
extern UCHAR ma_state;
#else
static const UCHAR all_master_errors = 0;
static const UCHAR ma_state = 0;
#endif
// 60 - 84 Geni Channel specific states
extern CHANNEL_PARAM channels[NO_OF_CHANNELS];
// 60 - 64 Bus channel
#if ( CTO_BUS_TYPE != Disable)
#define bus_ch_drv_state       channels[BUS_CH_INDX].drv_state
#define bus_ch_flags           channels[BUS_CH_INDX].flags
#else
static const UCHAR bus_ch_drv_state   = 0;
static const UCHAR bus_ch_flags       = 0;
#endif
// 65 - 69 Ir channel
#if ( CTO_IR_TYPE != Disable)
#define ir_ch_drv_state        channels[IR_CH_INDX].drv_state
#define ir_ch_flags            channels[IR_CH_INDX].flags
#else
static const UCHAR ir_ch_drv_state    = 0;
static const UCHAR ir_ch_flags        = 0;
#endif
// 70 - 74 COM channel
#if ( CTO_COM_TYPE != Disable)
#define com_ch_drv_state       channels[COM_CH_INDX].drv_state
#define com_ch_flags           channels[COM_CH_INDX].flags
#else
static const UCHAR com_ch_drv_state   = 0;
static const UCHAR com_ch_flags       = 0;
#endif
// 75 - 79 RS232 channel
#if ( CTO_RS232_TYPE != Disable)
#define rs232_ch_drv_state     channels[RS232_CH_INDX].drv_state
#define rs232_ch_flags         channels[RS232_CH_INDX].flags
#else
static const UCHAR rs232_ch_drv_state = 0;
static const UCHAR rs232_ch_flags     = 0;
#endif
// 80 - 84 PLM channel
#if ( CTO_PLM_TYPE != Disable)
#define plm_ch_drv_state       channels[PLM_CH_INDX].drv_state
#define plm_ch_flags           channels[PLM_CH_INDX].flags
#else
static const UCHAR plm_ch_drv_state   = 0;
static const UCHAR plm_ch_flags       = 0;
#endif
// 100 - 102 Geni Version
static const UCHAR major_ver         = (UCHAR)(GENI_VERSION_VAL >> 16);
static const UCHAR minor_ver         = (UCHAR)(GENI_VERSION_VAL >> 8);
static const UCHAR revision_ver      = (UCHAR) GENI_VERSION_VAL;
/****************************************************************************/
/* Class 0 table                                                            */
/****************************************************************************/
//  Data Item Pointer tables for GENIpro parameters  - must be placed here because it's using local vars
#if defined( __PC__ )
ID_PTR   vir_pro_tab[HIGH_PRO_ID + 1]  = {
#else
const ID_PTR   vir_pro_tab[HIGH_PRO_ID + 1]  = {
#endif
  0,0,                                                     /* 0,1  */
  (UCHAR *) &df_buf_len,                                   /* 2  */
  &unit_bus_mode,                                          /* 3  */
  (((UCHAR *)&operation_acc_ctr) + 1),                     /* 4  */
  ( UCHAR *)&operation_acc_ctr,                            /* 5  */
  0,0,0,0,                                                 /* 6,7,8,9 */
  0,                                                       /* 10 */
  (UCHAR *)&cto_class_8,                                   /* 11 */
  (UCHAR *)&cto_class_9,                                   /* 12 */
  (UCHAR *)&cto_class_10,                                  /* 13 */
  (UCHAR *)&cto_class_16_bit,                              /* 14 */
  0,0,0,0,0,                                               /* 15,16,17,18,19 */
  (UCHAR *)&bus_df_buf_len,                                /* 20 */
  (UCHAR *)&ir_df_buf_len,                                 /* 21 */
  (UCHAR *)&plm_df_buf_len,                                /* 22 */
  (UCHAR *)&rs232_df_buf_len,                              /* 23 */
  (UCHAR *)&com_df_buf_len,                                /* 24 */
  0,0,0,0,0,                                               /* 25,26,27,28,29 */
  (UCHAR *)&router_df_buf_len,                             /* 30 */
  (UCHAR *)&object_df_buf_len,                             /* 31 */
  (UCHAR *)&cto_buf_opt,                                   /* 32 */
  (UCHAR *)&buf_opt_ctr,                                   /* 33 */
  (UCHAR *)&cmd_buf_len,                                   /* 34 */
  (UCHAR *)&conf_buf_len,                                  /* 35 */
  (UCHAR *)&ref_buf_len,                                   /* 36 */
  (UCHAR *)&cto_buf16_opt,                                 /* 37 */
  (UCHAR *)&cto_buf32_opt,                                 /* 38 */
  (UCHAR *)&conf16_buf_len,                                /* 39 */
  (UCHAR *)&ref16_buf_len,                                 /* 40 */
  (UCHAR *)&conf32_buf_len,                                /* 41 */
  (UCHAR *)&ref32_buf_len,                                 /* 42 */
  0,0,0,0,0,0,0,                                           /* 43,44,45,46,47,48,49 */
  (((UCHAR *)&all_master_errors) + 1),                     /* 50 */
  ((UCHAR *)&all_master_errors),                           /* 51 */
  ((UCHAR *)&ma_state),                                    /* 52 */
  0,0,0,0,0,0,0,                                           /* 53,54,55,56,57,58,59 */
  (UCHAR *)&bus_ch_drv_state,                              /* 60 */
  (UCHAR *)&bus_ch_flags,                                  /* 61 */
  0,0,0,                                                   /* 62,63,64 */
  (UCHAR *)&ir_ch_drv_state,                               /* 65 */
  (UCHAR *)&ir_ch_flags,                                   /* 66 */
  0,0,0,                                                   /* 67,68,69 */
  (UCHAR *)&com_ch_drv_state,                              /* 70 */
  (UCHAR *)&com_ch_flags,                                  /* 71 */
  0,0,0,                                                   /* 72,73,74 */
  (UCHAR *)&rs232_ch_drv_state,                            /* 75 */
  (UCHAR *)&rs232_ch_flags,                                /* 76 */
  0,0,0,                                                   /* 77,78,79 */
  (UCHAR *)&plm_ch_drv_state,                              /* 80 */
  (UCHAR *)&plm_ch_flags,                                  /* 81 */
  0,0,0,                                                   /* 82,83,84 */
  0,0,0,0,0,                                               /* 85 - 89 */
  0,0,0,0,0,0,0,0,0,0,                                     /* 90 - 99 */
  (UCHAR *)&major_ver,                                     /* 100 */
  (UCHAR *)&minor_ver,                                     /* 101 */
  (UCHAR *)&revision_ver,                                  /* 102 */
  0,0,0,0,0,0,0,                                           /* 103 - 109 */
  (((UCHAR *)&tx_tgm_cnt) + 3),                            /* 110 */
  (((UCHAR *)&tx_tgm_cnt) + 2),                            /* 111 */
  (((UCHAR *)&tx_tgm_cnt) + 1),                            /* 112 */
  (((UCHAR *)&tx_tgm_cnt) + 0),                            /* 113 */
  (((UCHAR *)&rx_tgm_cnt) + 3),                            /* 114 */
  (((UCHAR *)&rx_tgm_cnt) + 2),                            /* 115 */
  (((UCHAR *)&rx_tgm_cnt) + 1),                            /* 116 */
  (((UCHAR *)&rx_tgm_cnt) + 0),                            /* 117 */
  (((UCHAR *)&crc_errors) + 1),                            /* 118 */
  (((UCHAR *)&crc_errors) + 0),                            /* 119 */
  (((UCHAR *)&data_errors) + 1),                           /* 120 */
  (((UCHAR *)&data_errors) + 0),                           /* 121 */
  (((UCHAR *)&master_timeout_errors) + 1),                 /* 122 */
  (((UCHAR *)&master_timeout_errors) + 0),                 /* 123 */
  (((UCHAR *)&slave_timeout_errors) + 1),                  /* 124 */
  (((UCHAR *)&slave_timeout_errors) + 0),                  /* 125 */
  (((UCHAR *)&rx_break_errors) + 1),                       /* 126 */
  (((UCHAR *)&rx_break_errors) + 0),                       /* 127 */
  (((UCHAR *)&geni_warning) + 1),                          /* 128 */
  (((UCHAR *)&geni_warning) + 0)                           /* 129 */
  };


/****************************************************************************/
/* Class 1 table                                                            */
/****************************************************************************/
#if defined( __PC__ )
ID_PTR vir_ch_tab[HIGH_CH_ID+1]  = { 0 };
#else
const ID_PTR vir_ch_tab[HIGH_CH_ID+1]  = { 0 };
#endif

UCHAR test_vir_0=10,test_vir_1=11,test_vir_2=12;

/*****************************************************************************/
/* CLASS 2, MEASURED DATA                                                    */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR  slave_meas_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1] = {
#else
const ID_PTR  slave_meas_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1] = {
#endif
{
   0, &test_vir_0, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
{
   0, &test_vir_1, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
{
   0, &test_vir_2, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
   NA, NA, NA, NA, NA, NA}};                         /*  250 - 255          */

#if defined( __PC__ )
ID_INFO  slave_meas_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1] = {
#else
const ID_INFO  slave_meas_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS_ID+1] = {
#endif
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
   NI, NI, NI, NI, NI, NI}};                         /* 250 - 255            */

/*****************************************************************************/
/* Class 3, Commands                                                         */
/*****************************************************************************/
#if defined( __PC__ )
ID_INFO  slave_cmd_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CMD_ID + 1] = {
#else
const ID_INFO  slave_cmd_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CMD_ID + 1] = {
#endif
{
   Unexecutable,                                     /*   0                  */
   Executable,                                       /*   1  RESET           */
   Executable,                                       /*   2  RESET_ALARM     */
   Executable,                                       /*   3  FACTORY_BOOT    */
   Executable,                                       /*   4  USER_BOOT       */
   Executable,                                       /*   5  STOP            */
   Executable,                                       /*   6  START           */
   Unexecutable,                                     /*   7                  */
   Unexecutable,                                     /*   8                  */
   Unexecutable,                                     /*   9                  */
   Unexecutable,                                     /*  10                  */
   Unexecutable,                                     /*  11                  */
   Unexecutable,                                     /*  12                  */
   Unexecutable,                                     /*  13                  */
   Unexecutable,                                     /*  14                  */
   Unexecutable,                                     /*  15                  */
   Unexecutable,                                     /*  16                  */
   Unexecutable,                                     /*  17                  */
   Unexecutable,                                     /*  18                  */
   Executable,                                       /*  19  USE             */
   Executable,                                       /*  20  TEST            */
   Unexecutable,                                     /*  21                  */
   Executable,                                       /*  22                  */
   Unexecutable,                                     /*  23                  */
   Unexecutable,                                     /*  24                  */
   Executable,                                       /*  25  MIN             */
   Executable,                                       /*  26  MAX             */
   Unexecutable,                                     /*  27                  */
   Unexecutable,                                     /*  28                  */
   Unexecutable,                                     /*  29                  */
   Executable,                                       /*  30  LOCK_KEYS       */
   Executable,                                       /*  31  UNLOCK_KEYS     */
   Unexecutable,                                     /*  32                  */
   Unexecutable,                                     /*  33                  */
   Unexecutable,                                     /*  34                  */
   Unexecutable,                                     /*  35                  */
   Unexecutable,                                     /*  36                  */
   Unexecutable,                                     /*  37                  */
   Unexecutable,                                     /*  38                  */
   Unexecutable,                                     /*  39                  */
   Unexecutable,                                     /*  40                  */
   Executable,                                       /*  41 AUTO_RESTART-ENABLE  */
   Executable,                                       /*  42 AUTO-RESTART-DISABLE */
   Unexecutable,                                     /*  43                  */
   Unexecutable,                                     /*  44                  */
   Unexecutable,                                     /*  45                  */
   Unexecutable,                                     /*  46                  */
   Unexecutable,                                     /*  47                  */
   Unexecutable,                                     /*  48                  */
   Unexecutable,                                     /*  49                  */
   Unexecutable,                                     /*  50                  */
   Unexecutable,                                     /*  51                  */
   Unexecutable,                                     /*  52                  */
   Unexecutable,                                     /*  53                  */
   Unexecutable,                                     /*  54                  */
   Unexecutable,                                     /*  55                  */
   Unexecutable,                                     /*  56                  */
   Unexecutable,                                     /*  57                  */
   Unexecutable,                                     /*  58                  */
   Unexecutable,                                     /*  59                  */
   Unexecutable,                                     /*  60                  */
   Unexecutable,                                     /*  61                  */
   Unexecutable,                                     /*  62                  */
   Unexecutable,                                     /*  63                  */
   Unexecutable,                                     /*  64                  */
   Unexecutable,                                     /*  65                  */
   Unexecutable,                                     /*  66                  */
   Executable,                                       /*  67 DOUBLE_STB_ENABLE*/
   Executable,                                       /*  68 DOUBLE_STB_DISABLE*/
   Unexecutable,                                     /*  69                  */
   Unexecutable,                                     /*  70                  */
   Unexecutable,                                     /*  71                  */
   Unexecutable,                                     /*  72                  */
   Unexecutable,                                     /*  73                  */
   Unexecutable,                                     /*  74                  */
   Unexecutable,                                     /*  75                  */
   Unexecutable,                                     /*  76                  */
   Unexecutable,                                     /*  77                  */
   Unexecutable},                                    /*  78                  */
{
   Unexecutable,                                     /*   0                  */
   Executable,                                       /*   1  RESET           */
   Executable,                                       /*   2  RESET_ALARM     */
   Executable,                                       /*   3  FACTORY_BOOT    */
   Executable,                                       /*   4  USER_BOOT       */
   Executable,                                       /*   5  STOP            */
   Executable,                                       /*   6  START           */
   Unexecutable,                                     /*   7                  */
   Unexecutable,                                     /*   8                  */
   Unexecutable,                                     /*   9                  */
   Unexecutable,                                     /*  10                  */
   Unexecutable,                                     /*  11                  */
   Unexecutable,                                     /*  12                  */
   Unexecutable,                                     /*  13                  */
   Unexecutable,                                     /*  14                  */
   Unexecutable,                                     /*  15                  */
   Unexecutable,                                     /*  16                  */
   Unexecutable,                                     /*  17                  */
   Unexecutable,                                     /*  18                  */
   Executable,                                       /*  19  USE             */
   Executable,                                       /*  20  TEST            */
   Unexecutable,                                     /*  21                  */
   Executable,                                       /*  22                  */
   Unexecutable,                                     /*  23                  */
   Unexecutable,                                     /*  24                  */
   Executable,                                       /*  25  MIN             */
   Executable,                                       /*  26  MAX             */
   Unexecutable,                                     /*  27                  */
   Unexecutable,                                     /*  28                  */
   Unexecutable,                                     /*  29                  */
   Executable,                                       /*  30  LOCK_KEYS       */
   Executable,                                       /*  31  UNLOCK_KEYS     */
   Unexecutable,                                     /*  32                  */
   Unexecutable,                                     /*  33                  */
   Unexecutable,                                     /*  34                  */
   Unexecutable,                                     /*  35                  */
   Unexecutable,                                     /*  36                  */
   Unexecutable,                                     /*  37                  */
   Unexecutable,                                     /*  38                  */
   Unexecutable,                                     /*  39                  */
   Unexecutable,                                     /*  40                  */
   Executable,                                       /*  41 AUTO_RESTART-ENABLE  */
   Executable,                                       /*  42 AUTO-RESTART-DISABLE */
   Unexecutable,                                     /*  43                  */
   Unexecutable,                                     /*  44                  */
   Unexecutable,                                     /*  45                  */
   Unexecutable,                                     /*  46                  */
   Unexecutable,                                     /*  47                  */
   Unexecutable,                                     /*  48                  */
   Unexecutable,                                     /*  49                  */
   Unexecutable,                                     /*  50                  */
   Unexecutable,                                     /*  51                  */
   Unexecutable,                                     /*  52                  */
   Unexecutable,                                     /*  53                  */
   Unexecutable,                                     /*  54                  */
   Unexecutable,                                     /*  55                  */
   Unexecutable,                                     /*  56                  */
   Unexecutable,                                     /*  57                  */
   Unexecutable,                                     /*  58                  */
   Unexecutable,                                     /*  59                  */
   Unexecutable,                                     /*  60                  */
   Unexecutable,                                     /*  61                  */
   Unexecutable,                                     /*  62                  */
   Unexecutable,                                     /*  63                  */
   Unexecutable,                                     /*  64                  */
   Unexecutable,                                     /*  65                  */
   Unexecutable,                                     /*  66                  */
   Executable,                                       /*  67 DOUBLE_STB_ENABLE*/
   Executable,                                       /*  68 DOUBLE_STB_DISABLE*/
   Unexecutable,                                     /*  69                  */
   Unexecutable,                                     /*  70                  */
   Unexecutable,                                     /*  71                  */
   Unexecutable,                                     /*  72                  */
   Unexecutable,                                     /*  73                  */
   Unexecutable,                                     /*  74                  */
   Unexecutable,                                     /*  75                  */
   Unexecutable,                                     /*  76                  */
   Unexecutable,                                     /*  77                  */
   Unexecutable},                                    /*  78                  */
{
   Unexecutable,                                     /*   0                  */
   Executable,                                       /*   1  RESET           */
   Executable,                                       /*   2  RESET_ALARM     */
   Executable,                                       /*   3  FACTORY_BOOT    */
   Executable,                                       /*   4  USER_BOOT       */
   Executable,                                       /*   5  STOP            */
   Executable,                                       /*   6  START           */
   Unexecutable,                                     /*   7                  */
   Unexecutable,                                     /*   8                  */
   Unexecutable,                                     /*   9                  */
   Unexecutable,                                     /*  10                  */
   Unexecutable,                                     /*  11                  */
   Unexecutable,                                     /*  12                  */
   Unexecutable,                                     /*  13                  */
   Unexecutable,                                     /*  14                  */
   Unexecutable,                                     /*  15                  */
   Unexecutable,                                     /*  16                  */
   Unexecutable,                                     /*  17                  */
   Unexecutable,                                     /*  18                  */
   Executable,                                       /*  19  USE             */
   Executable,                                       /*  20  TEST            */
   Unexecutable,                                     /*  21                  */
   Executable,                                       /*  22                  */
   Unexecutable,                                     /*  23                  */
   Unexecutable,                                     /*  24                  */
   Executable,                                       /*  25  MIN             */
   Executable,                                       /*  26  MAX             */
   Unexecutable,                                     /*  27                  */
   Unexecutable,                                     /*  28                  */
   Unexecutable,                                     /*  29                  */
   Executable,                                       /*  30  LOCK_KEYS       */
   Executable,                                       /*  31  UNLOCK_KEYS     */
   Unexecutable,                                     /*  32                  */
   Unexecutable,                                     /*  33                  */
   Unexecutable,                                     /*  34                  */
   Unexecutable,                                     /*  35                  */
   Unexecutable,                                     /*  36                  */
   Unexecutable,                                     /*  37                  */
   Unexecutable,                                     /*  38                  */
   Unexecutable,                                     /*  39                  */
   Unexecutable,                                     /*  40                  */
   Executable,                                       /*  41 AUTO_RESTART-ENABLE  */
   Executable,                                       /*  42 AUTO-RESTART-DISABLE */
   Unexecutable,                                     /*  43                  */
   Unexecutable,                                     /*  44                  */
   Unexecutable,                                     /*  45                  */
   Unexecutable,                                     /*  46                  */
   Unexecutable,                                     /*  47                  */
   Unexecutable,                                     /*  48                  */
   Unexecutable,                                     /*  49                  */
   Unexecutable,                                     /*  50                  */
   Unexecutable,                                     /*  51                  */
   Unexecutable,                                     /*  52                  */
   Unexecutable,                                     /*  53                  */
   Unexecutable,                                     /*  54                  */
   Unexecutable,                                     /*  55                  */
   Unexecutable,                                     /*  56                  */
   Unexecutable,                                     /*  57                  */
   Unexecutable,                                     /*  58                  */
   Unexecutable,                                     /*  59                  */
   Unexecutable,                                     /*  60                  */
   Unexecutable,                                     /*  61                  */
   Unexecutable,                                     /*  62                  */
   Unexecutable,                                     /*  63                  */
   Unexecutable,                                     /*  64                  */
   Unexecutable,                                     /*  65                  */
   Unexecutable,                                     /*  66                  */
   Executable,                                       /*  67 DOUBLE_STB_ENABLE*/
   Executable,                                       /*  68 DOUBLE_STB_DISABLE*/
   Unexecutable,                                     /*  69                  */
   Unexecutable,                                     /*  70                  */
   Unexecutable,                                     /*  71                  */
   Unexecutable,                                     /*  72                  */
   Unexecutable,                                     /*  73                  */
   Unexecutable,                                     /*  74                  */
   Unexecutable,                                     /*  75                  */
   Unexecutable,                                     /*  76                  */
   Unexecutable,                                     /*  77                  */
   Unexecutable}};                                   /*  78                  */

/*****************************************************************************/
/* Class 4, Configuration Parameters                                         */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_conf_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1] = {
#else
const ID_PTR   slave_conf_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1] = {
#endif
{
   0,  NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
{
   0,  NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
{
   0,  NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 0 - 9                */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
   NA, NA, NA, NA, NA, NA }};                         /* 250 - 255            */

#if defined( __PC__ )
ID_INFO  slave_conf_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1] = {
#else
const ID_INFO  slave_conf_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF_ID+1] = {
#endif
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
{
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
   NI, NI, NI, NI, NI, NI }};                        /* 250 - 255            */

/*****************************************************************************/
/* Class 5, Reference Values                                                 */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_ref_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1]   = {
#else
const ID_PTR   slave_ref_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1]   = {
#endif
{
   0,                                                /*  0                   */
   NA,                                               /*  1                   */
   NA },                                             /*  2                   */
{
   0,                                                /*  0                   */
   NA,                                               /*  1                   */
   NA },                                             /*  2                   */
{
   0,                                                /*  0                   */
   NA,                                               /*  1                   */
   NA }};                                            /*  2                   */

#if defined( __PC__ )
ID_INFO  slave_ref_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1]   = {
#else
const ID_INFO  slave_ref_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF_ID+1]   = {
#endif
{
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI },                                             /*  2                   */
{
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI },                                             /*  2                   */
{
   0,                                                /*  0                   */
   NI,                                               /*  1                   */
   NI }};                                            /*  2                   */


/*****************************************************************************/
/* Class 6: Test data                                                        */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR slave_test_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_TEST_ID+1] = {
#else
const ID_PTR slave_test_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_TEST_ID+1] = {
#endif
{
   0,                                                /*  ID   0              */
   NA,                                               /*  ID   1              */
   NA,                                               /*  ID   2              */
   NA,                                               /*  ID   3              */
   NA,                                               /*  ID   4              */
   NA },                                             /*  ID   5              */
{
   0,                                                /*  ID   0              */
   NA,                                               /*  ID   1              */
   NA,                                               /*  ID   2              */
   NA,                                               /*  ID   3              */
   NA,                                               /*  ID   4              */
   NA },                                             /*  ID   5              */
{
   0,                                                /*  ID   0              */
   NA,                                               /*  ID   1              */
   NA,                                               /*  ID   2              */
   NA,                                               /*  ID   3              */
   NA,                                               /*  ID   4              */
   NA }};                                            /*  ID   5              */


/*****************************************************************************/
/* Class 7, Ascii Strings                                                    */
/*****************************************************************************/
CHAR cl7_id10_u0[ASCII_SIZE] = "9998979695";

#if defined( __PC__ )
ID_PTR  slave_ascii_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_ASCII_ID+1] = {
#else
const ID_PTR  slave_ascii_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_ASCII_ID+1] = {
#endif
{
   0,                                                /* ID 0                 */
   (UCHAR *)"product name",                          /* ID 1 product name    */
   (UCHAR *)"project name",                          /* ID 2 project name    */
   (UCHAR *)"software name",                         /* ID 3 software_name   */
   __DATE__,                                         /* ID 4 compile_date    */
   (UCHAR *) GENI_VERSION_STR,                       /* ID 5 protocol_ver    */
   (UCHAR *)"grundfos no",                           /* ID 6 grundfos_no     */
   (UCHAR *)"developers",                            /* ID 7 developers      */
   (UCHAR *)"extra",                                 /* ID 8                 */
   (UCHAR *)"extra",                                 /* ID 9                 */
   (UCHAR *)&cl7_id10_u0,
   },
{
   0,                                                /* ID 0                 */
   (UCHAR *)"product name",                          /* ID 1 product name    */
   (UCHAR *)"project name",                          /* ID 2 project name    */
   (UCHAR *)"software name",                         /* ID 3 software_name   */
   __DATE__,                                         /* ID 4 compile_date    */
   (UCHAR *) GENI_VERSION_STR,                       /* ID 5 protocol_ver    */
   (UCHAR *)"grundfos no",                           /* ID 6 grundfos_no     */
   (UCHAR *)"developers",                            /* ID 7 developers      */
   (UCHAR *)"extra",                                 /* ID 8                 */
   (UCHAR *)"extra",                                 /* ID 9                 */
   },
{
   0,                                                /* ID 0                 */
   (UCHAR *)"product name",                          /* ID 1 product name    */
   (UCHAR *)"project name",                          /* ID 2 project name    */
   (UCHAR *)"software name",                         /* ID 3 software_name   */
   __DATE__,                                         /* ID 4 compile_date    */
   (UCHAR *) GENI_VERSION_STR,                       /* ID 5 protocol_ver    */
   (UCHAR *)"grundfos no",                           /* ID 6 grundfos_no     */
   (UCHAR *)"developers",                            /* ID 7 developers      */
   (UCHAR *)"extra",                                 /* ID 8                 */
   (UCHAR *)"extra",                                 /* ID 9                 */
   }};

/*****************************************************************************/
/* Class 8, Dump Memory                                                      */
/*****************************************************************************/
/* Table not declared                                                        */
/* const ID_PTR dump_tab[HIGH_MENU_ID+1] = {0};                              */

/*****************************************************************************/
/* CLASS 11, 16 BIT MEASURED DATA                                            */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR  slave_meas16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {0,0,0};
#else
const ID_PTR  slave_meas16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {0,0,0};
#endif
//const ID_PTR  slave_meas16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA}};                         /*  250 - 255          */

#if defined( __PC__ )
ID_INFO  slave_meas16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {0,0,0 };
#else
const ID_INFO  slave_meas16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {0,0,0 };
#endif
//const ID_INFO  slave_meas16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS16_ID+1] = {
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI}};                         /* 250 - 255            */

/*****************************************************************************/
/* CLASS 12, 16 BIT Configuration Parameters                                 */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_conf16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {0,0,0};
#else
const ID_PTR   slave_conf16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {0,0,0};
#endif
//const ID_PTR   slave_conf16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA }};                        /* 250 - 255            */
#if defined( __PC__ )
ID_INFO  slave_conf16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {0,0,0};
#else
const ID_INFO  slave_conf16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {0,0,0};
#endif
//const ID_INFO  slave_conf16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF16_ID+1] = {
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI }};                        /* 250 - 255            */

/*****************************************************************************/
/* Class 13, 16 BIT Reference Values                                         */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_ref16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = { 0,0,0 };
#else
const ID_PTR   slave_ref16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = { 0,0,0 };
#endif
//const ID_PTR   slave_ref16_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = {
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA }};                                            /*  2                   */

#if defined( __PC__ )
ID_INFO  slave_ref16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = {0,0,0};
#else
const ID_INFO  slave_ref16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = {0,0,0};
#endif
//const ID_INFO  slave_ref16_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF16_ID+1]   = {
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI }};                                            /*  2                   */

/*****************************************************************************/
/* CLASS 14, 32 BIT MEASURED DATA                                            */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR  slave_meas32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {0,0,0};
#else
const ID_PTR  slave_meas32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {0,0,0};
#endif
//const ID_PTR  slave_meas32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA},                          /*  250 - 255          */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /*  0 - 9              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  10 - 19            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  20 - 29            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  30 - 39            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  40 - 49            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  50 - 59            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  60 - 69            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  70 - 79            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  80 - 89            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  90 - 99            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  100 - 109          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  110 - 119          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  120 - 129          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  130 - 139          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  140 - 149          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  150 - 159          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  160 - 169          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  170 - 179          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  180 - 189          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  190 - 199          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  200 - 209          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  210 - 219          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  220 - 229          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  230 - 239          */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /*  240 - 249          */
//   NA, NA, NA, NA, NA, NA}};                         /*  250 - 255          */

#if defined( __PC__ )
ID_INFO  slave_meas32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {0,0,0 };
#else
const ID_INFO  slave_meas32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {0,0,0 };
#endif
//const ID_INFO  slave_meas32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_MEAS32_ID+1] = {
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI},                          /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9               */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99             */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239           */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249           */
//   NI, NI, NI, NI, NI, NI}};                         /* 250 - 255            */

/*****************************************************************************/
/* CLASS 15, 32 BIT Configuration Parameters                                 */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_conf32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {0,0,0};
#else
const ID_PTR slave_conf32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {0,0,0};
#endif
//const ID_PTR   slave_conf32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA },                         /* 250 - 255            */
//{
//   0, NA, NA, NA, NA, NA, NA, NA, NA, NA,            /* 0 - 9                */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 10 - 19              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 20 - 29              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 30 - 39              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 40 - 49              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 50 - 59              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 60 - 69              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 70 - 79              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 80 - 89              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 90 - 99              */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 100 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 110 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 120 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 130 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 140 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 150 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 160 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 170 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 180 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 190 - 109            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 200 - 209            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 210 - 219            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 220 - 229            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 230 - 239            */
//   NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,           /* 240 - 249            */
//   NA, NA, NA, NA, NA, NA }};                        /* 250 - 255            */
#if defined( __PC__ )
ID_INFO  slave_conf32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {0,0,0};
#else
const ID_INFO  slave_conf32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {0,0,0};
#endif
//const ID_INFO  slave_conf32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_CONF32_ID+1] = {
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI },                         /* 250 - 255            */
//{
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 0 - 9                */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 10 - 19              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 20 - 29              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 30 - 39              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 40 - 49              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 50 - 59              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 60 - 69              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 70 - 79              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 80 - 89              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 90 - 99              */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 100 - 109            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 110 - 119            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 120 - 129            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 130 - 139            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 140 - 149            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 150 - 159            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 160 - 169            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 170 - 179            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 180 - 189            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 190 - 199            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 200 - 209            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 210 - 219            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 220 - 229            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 230 - 239            */
//   NI, NI, NI, NI, NI, NI, NI, NI, NI, NI,           /* 240 - 249            */
//   NI, NI, NI, NI, NI, NI }};                        /* 250 - 255            */

/*****************************************************************************/
/* Class 16, 32 BIT Reference Values                                         */
/*****************************************************************************/
#if defined( __PC__ )
ID_PTR   slave_ref32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = { 0,0,0 };
#else
const ID_PTR   slave_ref32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = { 0,0,0 };
#endif
//const ID_PTR   slave_ref32_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = {
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NA,                                               /*  1                   */
//   NA }};                                            /*  2                   */

#if defined( __PC__ )
ID_INFO  slave_ref32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = {0,0,0};
#else
const ID_INFO  slave_ref32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = {0,0,0};
#endif
//const ID_INFO  slave_ref32_info_tab[MAX_VIR_SLAVE_COUNT][VIR_SLAVE_HIGH_REF32_ID+1]   = {
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI },                                             /*  2                   */
//{
//   0,                                                /*  0                   */
//   NI,                                               /*  1                   */
//   NI }};                                            /*  2                   */

/*****************************************************************************/
/* Vir_slave_pre_tab                                                         */
/*****************************************************************************/
#if defined( __PC__ )
PROTAB vir_slave_pre_tab[MAX_VIR_SLAVE_COUNT][17] = {
#else
const PROTAB vir_slave_pre_tab[MAX_VIR_SLAVE_COUNT][17] = {
#endif
{
{ HIGH_PRO_ID              ,                            0,         &vir_pro_tab[0],                        0,                           0},    // 0      Protocol table
{ HIGH_CH_ID               ,                            0,          &vir_ch_tab[0],                        0,                           0},    // 1      Bus table
{ VIR_SLAVE_HIGH_MEAS_ID   ,   &slave_meas_info_tab[0][0],  &slave_meas_tab[0][0] ,                        0,                           0},    // 2      Measurements
{ VIR_SLAVE_HIGH_CMD_ID    ,   &slave_cmd_info_tab[0][0] ,                       0,    &slave_cmd_buf[0][0] ,      VIR_SLAVE_CMD_BUF_LEN },    // 3      Commands
{ VIR_SLAVE_HIGH_CONF_ID   ,   &slave_conf_info_tab[0][0],  &slave_conf_tab[0][0] ,    &slave_conf_buf[0][0],     VIR_SLAVE_CONF_BUF_LEN },    // 4      Configurations
{ VIR_SLAVE_HIGH_REF_ID    ,   &slave_ref_info_tab[0][0] ,  &slave_ref_tab[0][0]  ,    &slave_ref_buf[0][0] ,      VIR_SLAVE_REF_BUF_LEN },    // 5      Reference
{ VIR_SLAVE_HIGH_TEST_ID   ,                            0,  &slave_test_tab[0][0] ,                        0,                           0},    // 6      Parameters for test
{ VIR_SLAVE_HIGH_ASCII_ID  ,                            0,  &slave_ascii_tab[0][0],   &slave_ascii_buf[0][0],     VIR_SLAVE_ASCII_BUF_LEN},    // 7      Ascii informations
{ VIR_SLAVE_HIGH_MEMORY_ID ,                            0,                       0,                        0,                           0},    // 8      Memory blocks       PGH
{                         0,                            0,                       0,                        0,                           0},    // 9      Routing class
{                         0,                            0,                       0,                        0,                           0},    // 10     Object
{ VIR_SLAVE_HIGH_MEAS16_ID , &slave_meas16_info_tab[0][0], &slave_meas16_tab[0][0],                        0,                           0},    // 11     16 bit measurements
{ VIR_SLAVE_HIGH_CONF16_ID , &slave_conf16_info_tab[0][0], &slave_conf16_tab[0][0],  &slave_conf16_buf[0][0],    VIR_SLAVE_CONF16_BUF_LEN},    // 12     16 bit configurations
{ VIR_SLAVE_HIGH_REF16_ID  , &slave_ref16_info_tab[0][0] , &slave_ref16_tab[0][0] ,  &slave_ref16_buf[0][0] ,     VIR_SLAVE_REF16_BUF_LEN},   // 13     16 bit reference
{ VIR_SLAVE_HIGH_MEAS32_ID , &slave_meas32_info_tab[0][0], &slave_meas32_tab[0][0],                        0,                           0},    // 14     32 bit measurements
{ VIR_SLAVE_HIGH_CONF32_ID , &slave_conf32_info_tab[0][0], &slave_conf32_tab[0][0],  &slave_conf32_buf[0][0],    VIR_SLAVE_CONF32_BUF_LEN},    // 15     32 bit configurations
{ VIR_SLAVE_HIGH_REF32_ID  , &slave_ref32_info_tab[0][0] , &slave_ref32_tab[0][0] ,  &slave_ref32_buf[0][0] ,     VIR_SLAVE_REF32_BUF_LEN}},   // 16     32 bit reference
{
{ HIGH_PRO_ID              ,                            0,         &vir_pro_tab[0],                        0,                           0},    // 0      Protocol table
{ HIGH_CH_ID               ,                            0,          &vir_ch_tab[0],                        0,                           0},    // 1      Bus table
{ VIR_SLAVE_HIGH_MEAS_ID   ,   &slave_meas_info_tab[1][0],  &slave_meas_tab[1][0] ,                        0,                           0},    // 2      Measurements
{ VIR_SLAVE_HIGH_CMD_ID    ,   &slave_cmd_info_tab[1][0] ,                       0,    &slave_cmd_buf[1][0] ,      VIR_SLAVE_CMD_BUF_LEN },    // 3      Commands
{ VIR_SLAVE_HIGH_CONF_ID   ,   &slave_conf_info_tab[1][0],  &slave_conf_tab[1][0] ,    &slave_conf_buf[1][0],     VIR_SLAVE_CONF_BUF_LEN },    // 4      Configurations
{ VIR_SLAVE_HIGH_REF_ID    ,   &slave_ref_info_tab[1][0] ,  &slave_ref_tab[1][0]  ,    &slave_ref_buf[1][0] ,      VIR_SLAVE_REF_BUF_LEN },    // 5      Reference
{ VIR_SLAVE_HIGH_TEST_ID   ,                            0,  &slave_test_tab[1][0] ,                        0,                           0},    // 6      Parameters for test
{ VIR_SLAVE_HIGH_ASCII_ID  ,                            0,  &slave_ascii_tab[1][0],   &slave_ascii_buf[1][0],     VIR_SLAVE_ASCII_BUF_LEN},    // 7      Ascii informations
{ VIR_SLAVE_HIGH_MEMORY_ID ,                            0,                       0,                        0,                           0},    // 8      Memory blocks       PGH
{                         0,                            0,                       0,                        0,                           0},    // 9      Routing class
{                         0,                            0,                       0,                        0,                           0},    // 10     Object
{ VIR_SLAVE_HIGH_MEAS16_ID , &slave_meas16_info_tab[1][0], &slave_meas16_tab[1][0],                        0,                           0},    // 11     16 bit measurements
{ VIR_SLAVE_HIGH_CONF16_ID , &slave_conf16_info_tab[1][0], &slave_conf16_tab[1][0],  &slave_conf16_buf[1][0],    VIR_SLAVE_CONF16_BUF_LEN},    // 12     16 bit configurations
{ VIR_SLAVE_HIGH_REF16_ID  , &slave_ref16_info_tab[1][0] , &slave_ref16_tab[1][0] ,  &slave_ref16_buf[1][0] ,     VIR_SLAVE_REF16_BUF_LEN},   // 13     16 bit reference
{ VIR_SLAVE_HIGH_MEAS32_ID , &slave_meas32_info_tab[1][0], &slave_meas32_tab[1][0],                        0,                           0},    // 14     32 bit measurements
{ VIR_SLAVE_HIGH_CONF32_ID , &slave_conf32_info_tab[1][0], &slave_conf32_tab[1][0],  &slave_conf32_buf[1][0],    VIR_SLAVE_CONF32_BUF_LEN},    // 15     32 bit configurations
{ VIR_SLAVE_HIGH_REF32_ID  , &slave_ref32_info_tab[1][0] , &slave_ref32_tab[1][0] ,  &slave_ref32_buf[1][0] ,     VIR_SLAVE_REF32_BUF_LEN}},   // 16     32 bit reference
{
{ HIGH_PRO_ID              ,                            0,         &vir_pro_tab[0],                        0,                           0},    // 0      Protocol table
{ HIGH_CH_ID               ,                            0,          &vir_ch_tab[0],                        0,                           0},    // 1      Bus table
{ VIR_SLAVE_HIGH_MEAS_ID   ,   &slave_meas_info_tab[2][0],  &slave_meas_tab[2][0] ,                        0,                           0},    // 2      Measurements
{ VIR_SLAVE_HIGH_CMD_ID    ,   &slave_cmd_info_tab[2][0] ,                       0,    &slave_cmd_buf[2][0] ,      VIR_SLAVE_CMD_BUF_LEN },    // 3      Commands
{ VIR_SLAVE_HIGH_CONF_ID   ,   &slave_conf_info_tab[2][0],  &slave_conf_tab[2][0] ,    &slave_conf_buf[2][0],     VIR_SLAVE_CONF_BUF_LEN },    // 4      Configurations
{ VIR_SLAVE_HIGH_REF_ID    ,   &slave_ref_info_tab[2][0] ,  &slave_ref_tab[2][0]  ,    &slave_ref_buf[2][0] ,      VIR_SLAVE_REF_BUF_LEN },    // 5      Reference
{ VIR_SLAVE_HIGH_TEST_ID   ,                            0,  &slave_test_tab[2][0] ,                        0,                           0},    // 6      Parameters for test
{ VIR_SLAVE_HIGH_ASCII_ID  ,                            0,  &slave_ascii_tab[2][0],   &slave_ascii_buf[2][0],     VIR_SLAVE_ASCII_BUF_LEN},    // 7      Ascii informations
{ VIR_SLAVE_HIGH_MEMORY_ID ,                            0,                       0,                        0,                           0},    // 8      Memory blocks       PGH
{                         0,                            0,                       0,                        0,                           0},    // 9      Routing class
{                         0,                            0,                       0,                        0,                           0},    // 10     Object
{ VIR_SLAVE_HIGH_MEAS16_ID , &slave_meas16_info_tab[2][0], &slave_meas16_tab[2][0],                        0,                           0},    // 11     16 bit measurements
{ VIR_SLAVE_HIGH_CONF16_ID , &slave_conf16_info_tab[2][0], &slave_conf16_tab[2][0],  &slave_conf16_buf[2][0],    VIR_SLAVE_CONF16_BUF_LEN},    // 12     16 bit configurations
{ VIR_SLAVE_HIGH_REF16_ID  , &slave_ref16_info_tab[2][0] , &slave_ref16_tab[2][0] ,  &slave_ref16_buf[2][0] ,     VIR_SLAVE_REF16_BUF_LEN},    // 13     16 bit reference
{ VIR_SLAVE_HIGH_MEAS32_ID , &slave_meas32_info_tab[2][0], &slave_meas32_tab[2][0],                        0,                           0},    // 14     32 bit measurements
{ VIR_SLAVE_HIGH_CONF32_ID , &slave_conf32_info_tab[2][0], &slave_conf32_tab[2][0],  &slave_conf32_buf[2][0],    VIR_SLAVE_CONF32_BUF_LEN},    // 15     32 bit configurations
{ VIR_SLAVE_HIGH_REF32_ID  , &slave_ref32_info_tab[2][0] , &slave_ref32_tab[2][0] ,  &slave_ref32_buf[2][0] ,     VIR_SLAVE_REF32_BUF_LEN}}};  // 16     32 bit reference

