#ifndef gfFramework_PCComDriver_h
#define gfFramework_PCComDriver_h


#include <windows.h>
#include <string>


class PCComDriver {
public:
    PCComDriver();
    //PCComDriver(const char* port, unsigned int baud = 9600, unsigned int stopBits);
    ~PCComDriver();

    bool OpenPort(const char* port, unsigned int baud,unsigned int parity,unsigned int stopBits);
    void ClosePort();

    bool SendData(unsigned char data);
    bool SendData(const unsigned char* data, unsigned int len);

    void GetData(unsigned char* destBuf, unsigned int* len);

    const char* GetErrorString() const;

    bool IsValid() const;
    bool IsOk() const;

    bool IsPortOpen() const;

private:
    void InitializeOverlapped();
    // State
    HANDLE      mPortHandle; /**< Handle of the COM port. */
    bool        mValid; /**< Set by the constructor after successful construction. */
    bool        mOk; /**< Set to 'false' if an error occurs. A description is saved in mErrorString. */
    std::string mErrorString; /**< A string describing the error that caused mOk to become 'false'. */  
    OVERLAPPED  mOverlappedWait;
    OVERLAPPED  mOverlappedWrite;

};


#endif  // gfFramework_PCComDriver_h