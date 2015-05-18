#ifndef __pc_com_driver_h__
#define __pc_com_driver_h__


#ifdef __cplusplus  
  extern "C" { 
#endif 

    void SendComData(const unsigned char* data, unsigned int len);

    unsigned int ReceiveComData(unsigned char* destBuf, unsigned int maxCount);   

    unsigned char StartComTask(void);

#ifdef __cplusplus    
  }  
#endif // __cplusplus  

#endif // __pc_com_driver_h__