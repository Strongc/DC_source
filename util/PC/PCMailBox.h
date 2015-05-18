// pcmailbox.h

#ifndef pcMailbox_h
#define pcMailbox_h

#include <assert.h>


class CountedSemaphore {
public:
    CountedSemaphore(long initial,long count) { semaphore = CreateSemaphore(NULL,initial,count,NULL); }
    ~CountedSemaphore() { CloseHandle(semaphore); }
    void P()       { WaitForSingleObject(semaphore,INFINITE); }
    void V()       { ReleaseSemaphore(semaphore,1,NULL); }
    void Reset(long initial); 
private:
    HANDLE semaphore;
};

inline void CountedSemaphore::Reset(long initial) {
    if (0==initial) {
        // Tælle ned
        while (WaitForSingleObject(semaphore,0) != WAIT_TIMEOUT);
    } else {
        // Tælle op
        while (ReleaseSemaphore(semaphore,1,NULL));
    }
}

class PCMonitor {
public:
    PCMonitor()          { OS_CREATERSEMA(&mutex); }
    virtual ~PCMonitor() {  }
    class Lock {
    public:
        Lock(PCMonitor& m) : m(m) { OS_Use(&m.mutex); }
        ~Lock()                   { OS_Unuse(&m.mutex); }
    private:
        PCMonitor& m;
        // Prevent
        Lock(const Lock&);
        Lock& operator = (const Lock&);
    };
private:
    OS_RSEMA mutex;
    // prevent
    PCMonitor(const PCMonitor&);
    PCMonitor& operator = (const PCMonitor&);
friend class Lock;
};


template <class T>
class Singleton { 
public:
    static T& TheInstance();
protected:
    Singleton() {}   
private:
    // Prevent
    Singleton(const Singleton&);
    Singleton& operator = (const Singleton&);
};

template <class T>
inline T& Singleton<T>::TheInstance() {
    static T theInstance;
    return theInstance;
}


class PC_TX_MSG {
public:
    void Set(unsigned char* msg) {
        memcpy(buffer,msg,sizeof(buffer));
    }
    void Get(unsigned char* msg) {
        memcpy(msg,buffer,sizeof(buffer));
    }
    unsigned char buffer[TXTGM_LEN];
};


class PC_RX_MSG {
public:
    void Set(unsigned char* msg) {
        memcpy(buffer,msg,sizeof(buffer));
    }
    void Get(unsigned char* msg) {
        memcpy(msg,buffer,sizeof(buffer));
    }
    unsigned char buffer[RXTGM_LEN];
};


template <class T, int size>
class PC_MailBox : private PCMonitor, public Singleton<PC_MailBox<T,size> > {
public: 
    void Reset() {
        released = false;
        put      = size-1;
        get      = 0;
        count    = 0;
        space.Reset(size);
        items.Reset(0);
    }
    void Release() {
        Lock lock(*this);
        released = true;
        items.V();
        space.V();
    }
    bool Put(unsigned char* msg) {       
        space.P();
        Lock lock(*this); 
        if (released) return false;       
        assert(count < size); 
        assert(count >= 0);
        ++put %= size;
        messages[put].Set(msg);       
        count++;       
        items.V();        
        return true;
    }
    bool Get(unsigned char* msg) {               
        items.P();
        Lock lock(*this);
        if (released) return false;
        assert(count > 0);
        assert(count <= size);                
        messages[get].Get(msg);       
        ++get %= size;
        count--;       
        space.V();       
        return true;
    }
private:  
    PC_MailBox() : space(size,size), items(0,size), released(false), put(size-1), get(0), count(0) {}
    T           messages[size];
    int         put;
    int         get;       
    int         count; // sanity check
    CountedSemaphore space;
    CountedSemaphore items;
    bool        released;
friend class Singleton<PC_MailBox<T,size> >;
};


extern PC_MailBox<PC_TX_MSG,10>& transmitBox;
extern PC_MailBox<PC_RX_MSG,10>& receiveBox;


#endif // pcMailbox_h
