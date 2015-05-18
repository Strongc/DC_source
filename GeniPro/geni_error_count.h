#ifndef __geni_error_count_h__
#define __geni_error_count_h__


#ifdef __cplusplus
  extern "C" {
#endif //


extern unsigned int all_master_errors; 
extern unsigned int dir_timeout_errors;
extern unsigned int auto_timeout_errors;
extern unsigned int rx_break_header_errors;
extern unsigned int rx_break_receiving_errors;
//extern unsigned int crc_errors; 

extern void ResetGeniErrorCount();

#ifdef __cplusplus
  }
#endif // __cplusplus

#endif //__geni_error_count_h__
