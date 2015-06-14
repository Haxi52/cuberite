
// IsThread.h

// Interfaces to the cIsThread class representing an OS-independent wrapper for a class that implements a thread.
// This class will eventually suupersede the old cThread class

/*
Usage:
To have a new thread, declare a class descending from cIsClass.
Then override its Execute() method to provide your thread processing.
In the descending class' constructor call the Start() method to start the thread once you're finished with initialization.
*/





#pragma once
#include <thread>





class cIsThread
{
protected:
	/** This is the main thread entrypoint.
	This function, overloaded by the descendants, is called in the new thread. */
	virtual void Execute(void) = 0;

	/** The overriden Execute() method should check this value periodically and terminate if this is true. */
	volatile bool m_ShouldTerminate;

public:
	cIsThread(const AString & a_ThreadName);
	virtual ~cIsThread();

	/** Starts the thread; returns without waiting for the actual start. */
	bool Start(void);

	/** Signals the thread to terminate and waits until it's finished. */
	void Stop(void);

	/** Waits for the thread to finish. Doesn't signalize the ShouldTerminate flag. */
	bool Wait(void);

	/** Returns true if the thread calling this function is the thread contained within this object. */
	bool IsCurrentThread(void) const { return std::this_thread::get_id() == m_Thread.get_id(); }
	
private:
	/** Wrapper for Execute() that waits for the initialization event, to prevent race conditions in thread initialization. */
	void DoExecute(void);

	AString m_ThreadName;
	std::thread m_Thread;

	/** The event that is used to wait with the thread's execution until the thread object is fully initialized.
	This prevents the IsCurrentThread() call to fail because of a race-condition where the thread starts before m_Thread has been fully assigned.
	Note: dependant on the cEvent behaviour of a call to Wait() not blocking if Set() was called prior with no threads waiting
	m_Thread can initialise faster than (and call Set() before) the child thread can be created. */
	cEvent m_evtStart;
} ;




