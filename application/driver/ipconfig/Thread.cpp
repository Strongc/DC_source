/** \file Thread.cpp
 *
 * Written by: Jesper Larsen, jla@iotech.dk
 * Company: IO Technologies A/S, Egaa, Denmark
 *
 * Project: Grundfos MPC DHCP / IP configuration
 * Projectno.: 5072
 * File: Thread.cpp
 *
 * $Log: Thread.cpp,v $
 * Revision 1.1.2.7  2005/05/26 09:30:43  jla
 * Added struct contaning OS_TASKID and Thread pointers for housekeeping. Fix new call bug
 *
 * Revision 1.1.2.6  2005/05/20 14:35:58  jla
 * Now uses STL lists for housekeeping. Implemented terminate, suspend and resume
 *
 * Revision 1.1.2.5  2005/05/20 13:33:55  jla
 * Implementing list with threads
 *
 * Revision 1.1.2.4  2005/05/14 09:00:15  jla
 * Removed cout
 *
 * Revision 1.1.2.3  2005/05/12 07:24:38  jla
 * Added newline...
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

#include "Thread.h"

list<TaskIdentifier> Thread::mtaskid;
unsigned int Thread::mtaskno = 0;


Thread::Thread()
 : mrunning(false),
 	mpstack(0)
{
	if (Thread::mtaskno < THREADMAXNO)
	{
		TaskIdentifier ti;
		ti.pthread = this;
		ti.taskid = &mtaskcontrol;

		mtaskid.push_back(ti);
		mtaskno++;
	}

	//cout << "Running threads: " << Thread::mtaskno << endl;
}

/**
 */
Thread::~Thread()
{
	terminate();
	if (mpstack)
		delete[] mpstack;

	list<TaskIdentifier>::iterator j;

	for (j=mtaskid.begin() ; j!=mtaskid.end() ; j++)
	{
		if (this == (*j).pthread)
		{
			mtaskid.erase(j);
			break;
		}
	}

	mtaskno--;
}

/**
 * This method is called by OS_CreateTask, when start method is called.
 * threadMapper then calls the run method for the actual object.
 */
void Thread::threadMapper()
{
	Thread *p = getThreadPtr();

	if (p)
		p->run();
}

/**
 * This method find a pointer for the current running Thread.
 * The OS_GetTaskID function is used to get a pointer to the OS_TASK
 * control structure for the current running thread. The mtaskid list is
 * then searched for a matching pointer. If a matching pointer is found, the
 * corresponding Thread pointer is returned from the mpthread list.
 *
 * \return Pointer to Thread object if found. 0 is returned if not found.
 */
Thread* Thread::getThreadPtr()
{
	OS_TASKID thistask = OS_GetTaskID();
	Thread* pthread = 0;

	list<TaskIdentifier>::iterator j;

	for (j=mtaskid.begin() ; j!=mtaskid.end() ; j++)
	{
		if (thistask == (*j).taskid)
		{
			pthread = (*j).pthread;
			break;
		}
	}

	return pthread;
}

/**
 * \param priority Priority of the thread.
 * \param name Null-terminated string with thread name
 * \param stacksize Stacksize in byte for the thread.
 */
void Thread::start( unsigned int priority,
					const char *name,
					unsigned int stacksize)
{
	if (mpstack==0)
	{
		mpstack = new unsigned char [stacksize];
	}

    OS_CreateTask(  &mtaskcontrol,          //OS_TASK* pTask,
					name, 					//char* pName,
					priority, 				//unsigned char Priority,
					threadMapper,			//voidRoutine* pRoutine,
					mpstack,				//void* pStack,
					stacksize,				//unsigned StackSize,
					STACKTIMESLICE);		//unsigned TimeSlice);

	mrunning = true;
}


void Thread::suspend()
{
	OS_Suspend(&mtaskcontrol);
}

/**
 * Only call this method after suspending the thread
 */
void Thread::resume()
{
	OS_Resume(&mtaskcontrol);
}


void Thread::terminate()
{
	mrunning = false;

	if (getThreadPtr() == this)			//Called from within thread to terminate?
	{
		OS_Terminate(0);
	}
	else
	{
		OS_Terminate(&mtaskcontrol);
	}
}

/**
 * \param delay Delay in "basic time intervals". See embOS documentation.
 */
void Thread::delay(unsigned int delay)
{
	 OS_Delay(delay);
}
