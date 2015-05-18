// PCComDriver.cpp

#include "PCComDriver.h"
#include <assert.h>

#ifdef __PC__
#include <atlconv.h>
#endif

PCComDriver::PCComDriver() :
    mErrorString(""),
    mPortHandle(NULL),
    mValid(false),
    mOk(false) 
{
    memset(&mOverlappedWait, 0, sizeof(mOverlappedWait));
    memset(&mOverlappedWrite, 0, sizeof(mOverlappedWrite));
    mValid = true;  
}

PCComDriver::~PCComDriver()
{
    ClosePort();
    if (mOverlappedWait.hEvent) CloseHandle(mOverlappedWait.hEvent);
    if (mOverlappedWrite.hEvent) CloseHandle(mOverlappedWrite.hEvent);
}

bool PCComDriver::OpenPort(const char* port, unsigned int baud,unsigned int parity, unsigned int stopBits)
{        
    if (!mValid) return false;

    ClosePort();   
#ifndef __PC__        
    mPortHandle = CreateFile(
        port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
else
    mPortHandle = CreateFile(
        A2T(port),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

#endif

    if (mPortHandle == INVALID_HANDLE_VALUE) {
        mPortHandle = NULL;
        mErrorString = "Error opening COMport";
        mOk = false;
        return false;
    }

    if (! ::SetCommMask(mPortHandle,EV_RXCHAR)) {
		mErrorString = "Error setting comm mask";
        mOk = false;
        return false;	
    }

    DCB dcb; 
    if (!GetCommState(mPortHandle, &dcb)) {
        CloseHandle(mPortHandle);
        mPortHandle = NULL;
        mErrorString = "Error getting CommState";
        mOk = false;
        return false;
    }

    dcb.BaudRate = CBR_9600;      // set the baud rate
    dcb.ByteSize = 8;             // data size, xmit, and rcv
    dcb.Parity = NOPARITY;        // no parity bit
    dcb.StopBits = ONESTOPBIT;    // one stop bit

    if (!SetCommState(mPortHandle, &dcb)) {
        unsigned int error = GetLastError();
        CloseHandle(mPortHandle);
        mPortHandle = NULL;
        mErrorString = "Error setting CommState";
        mOk = false;
        return false;
    }

    COMMTIMEOUTS timeouts;

    if (!GetCommTimeouts(mPortHandle, &timeouts)) {
        CloseHandle(mPortHandle);
        mPortHandle = NULL;
        mErrorString = "Error getting CommTimeouts";
        mOk = false;
        return false;
    }
    
    timeouts.ReadIntervalTimeout			= MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier		= 0;
	timeouts.ReadTotalTimeoutConstant		= 0;
	timeouts.WriteTotalTimeoutMultiplier	= 0;
	timeouts.WriteTotalTimeoutConstant		= 0;
    
    if (!SetCommTimeouts(mPortHandle, &timeouts)) {
        CloseHandle(mPortHandle);
        mPortHandle = NULL;
        mErrorString = "Error setting CommTimeouts";
        mOk = false;
        return false;
    }

    InitializeOverlapped();

    return mOk = true;
}

void PCComDriver::ClosePort() {
    if (mPortHandle != NULL) CloseHandle(mPortHandle);
}


const char* PCComDriver::GetErrorString() const
{
    return mErrorString.c_str();
}


bool PCComDriver::IsValid() const
{
    return mValid;
}

bool PCComDriver::IsOk() const
{
    return mValid && mOk;
}

bool PCComDriver::IsPortOpen() const
{
    return mValid && mOk && mPortHandle != NULL;
}

bool PCComDriver::SendData(unsigned char data) {
    if (mOverlappedWrite.hEvent) ResetEvent(mOverlappedWrite.hEvent);
    DWORD written_count;
	if (WriteFile(mPortHandle, &data, 1, &written_count, &mOverlappedWrite)) {
        WaitForSingleObject(mOverlappedWrite.hEvent ,INFINITE);
        return true;
    } 
    return false;
}

bool PCComDriver::SendData(const unsigned char* data, unsigned int len) {
    if (mOverlappedWrite.hEvent) ResetEvent(mOverlappedWrite.hEvent);

    DWORD written_count;
    if (WriteFile(mPortHandle, data, len, &written_count, &mOverlappedWrite)) {
        WaitForSingleObject(mOverlappedWrite.hEvent ,INFINITE);
        return true;
    }
    return false;
}


void PCComDriver::GetData(unsigned char* destBuf, unsigned int* len) {
    *len = 0;
    if (mOverlappedWait.hEvent) ResetEvent(mOverlappedWait.hEvent); 
    // Setup the wait event
    DWORD dwEvtMask;
    if (!WaitCommEvent(mPortHandle, &dwEvtMask, &mOverlappedWait)) {
        unsigned int error = GetLastError();
		assert(error == ERROR_IO_PENDING);
	}
    // Wait until a data arrived
    WaitForSingleObject(mOverlappedWait.hEvent ,INFINITE);
   
    if (dwEvtMask & EV_RXCHAR) {
        DWORD dwBytesRead = 0;
		OVERLAPPED ovRead;
		memset(&ovRead,0,sizeof(ovRead));
		ovRead.hEvent = CreateEvent( 0,true,0,0);
        unsigned char* buffer = destBuf;
		do {
			ResetEvent(ovRead.hEvent);
			char szTmp[128];		
			memset(szTmp,0,sizeof(szTmp));
			if (!ReadFile(mPortHandle,szTmp,sizeof(szTmp),&dwBytesRead,&ovRead)) break;	           
			if ( dwBytesRead > 0 )
			{
				// Add data to the buffer               
                memcpy(buffer,szTmp,dwBytesRead);               
				(*len) += dwBytesRead;
                buffer = &destBuf[*len];
			}
		}while (dwBytesRead > 0 );
		CloseHandle(ovRead.hEvent );        
    }
    ResetEvent (mOverlappedWait.hEvent );    
}

void PCComDriver::InitializeOverlapped()
{
    if (mOverlappedWait.hEvent) CloseHandle(mOverlappedWait.hEvent);
    if (mOverlappedWrite.hEvent) CloseHandle(mOverlappedWrite.hEvent);
    memset(&mOverlappedWait, 0, sizeof(mOverlappedWait));
    memset(&mOverlappedWrite, 0, sizeof(mOverlappedWrite));
    mOverlappedWait.hEvent = ::CreateEvent(0, true, false, 0);
    mOverlappedWrite.hEvent = ::CreateEvent(0, true, false, 0);
}

 // PCComDriver.cpp
