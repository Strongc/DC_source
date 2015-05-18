// pcmailbox.cpp

#include <iobcomdrv.h>
#include <pcmailbox.h>



PC_MailBox<PC_TX_MSG,10>& transmitBox = PC_MailBox<PC_TX_MSG,10>::TheInstance();
PC_MailBox<PC_RX_MSG,10>& receiveBox  = PC_MailBox<PC_RX_MSG,10>::TheInstance();


extern "C" void ReleaseMailBoxes(void)
{   
    transmitBox.Release();
    receiveBox.Release();
}


    
// pcmailbox.cpp