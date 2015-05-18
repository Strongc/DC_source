/** \file Thread.h
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: Thread.h
 *
 * $Log: Thread.h,v $
 * Revision 1.1.2.6  2005/06/27 07:59:57  jla
 * Updated documentation
 *
 * Revision 1.1.2.5  2005/05/26 09:30:43  jla
 * Added struct contaning OS_TASKID and Thread pointers for housekeeping. Fix new call bug
 *
 * Revision 1.1.2.4  2005/05/20 14:35:58  jla
 * Now uses STL lists for housekeeping. Implemented terminate, suspend and resume
 *
 * Revision 1.1.2.3  2005/05/20 13:33:55  jla
 * Implementing list with threads
 *
 * Revision 1.1.2.2  2005/05/09 13:14:57  jla
 * Added delay method
 *
 * Revision 1.1.2.1  2005/05/03 15:56:26  jla
 * Påbegyndt udvikling på den "rigtige" compiler og rtip
 *
 * Revision 1.1  2005/04/12 14:13:09  jla
 * Scratch version
 *
 */

#ifndef _THREAD_H_
#define _THREAD_H_

#include <RTOS.H>

#include <list>
#include <algorithm>
#include <iostream>

using namespace std;

#define THREADSTACKSIZE		1024
#define	THREADMAXNO			16
#define	STACKTIMESLICE		10

class Thread;

typedef  struct
{
	OS_TASKID taskid;
	Thread* pthread;
} TaskIdentifier;

/**\brief Encapsulates embOS's thread/task functionality
 *
 */
class Thread
{
public:
	Thread();
	virtual ~Thread();

	void start( unsigned int priority,
				const char *name="",
				unsigned int stacksize=THREADSTACKSIZE);	//!<Start the thread
	void suspend();							//!<Suspend the thread
	void resume();							//!<Resume the thread
	void terminate();						//!<Terminate/stop the thread

	bool isRunning() {return mrunning;}		//!<Return true if thread is running

	static void delay(unsigned int);		//!<Delay the current executing thread

protected:
	virtual void run()=0;					//!<This method must contain functionality of thread

private:
    static void threadMapper();				//!<Called by embOS OS_CreateTask
	static Thread* getThreadPtr();			//!<Get pointer to current executing thread

	static list<TaskIdentifier> mtaskid;	//!<List of pointers to OS_TASK and Threads, see getThreadPtr
	static unsigned int mtaskno;			//!<Number of instances of Thread

	OS_TASK mtaskcontrol;					//!<embOS internal task control structure
	unsigned char* mpstack;					//!<Pointer to stack for thread

	volatile bool mrunning;					//!<True if thread has been started, and not terminated
};

#endif //_THREAD_H_
