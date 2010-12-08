/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#include "atomicint.h"

#ifndef AX_THREAD_H 
#define AX_THREAD_H

AX_BEGIN_NAMESPACE


#define AX_INFINITE 0xFFFFFFFF


//------------------------------------------------------------------------------
// class SyncObject
//------------------------------------------------------------------------------
struct AX_API SyncObject {
	virtual ~SyncObject() {}

	virtual bool lock(uint_t timeout = AX_INFINITE) = 0;
	virtual bool unlock() = 0;
};


//------------------------------------------------------------------------------
// class SyncMutex
//------------------------------------------------------------------------------
class AX_API SyncMutex {
public:
	SyncMutex();
	virtual ~SyncMutex();

	virtual bool lock(uint_t timeout = AX_INFINITE);
	virtual bool unlock();

private:
	mutable void *m_object;
};

//------------------------------------------------------------------------------
// class auto lock
//------------------------------------------------------------------------------
#define SCOPE_LOCK ScopeLock __scopeLocker(m_mutex);


class AX_API ThreadSafe {
public:
	mutable SyncMutex m_mutex;
};

class ScopeLock {
public:
	ScopeLock(SyncMutex &syncobject) : m_mutex(syncobject) { m_mutex.lock(); }
	~ScopeLock() { m_mutex.unlock(); }
private:
	SyncMutex &m_mutex;
};



//------------------------------------------------------------------------------
// class SyncEvent
//------------------------------------------------------------------------------
class AX_API SyncEvent {
public:
	SyncEvent();
	virtual ~SyncEvent();

	virtual bool lock(uint_t timeout = AX_INFINITE);
	virtual bool unlock();

	bool setEvent();
	bool pulseEvent();
	bool resetEvent();

private:
	void *m_object;
};



//------------------------------------------------------------------------------
// class Thread
//------------------------------------------------------------------------------
class INotifyHandler;
class AX_API Thread {
public:
	Thread();
	virtual ~Thread();

	void startThread();
	void stopThread();
	bool isCurrentThread() const;

	void addAsyncNotify(INotifyHandler *handler, int index);

	virtual void doRun() = 0;		// work entry

private:
	Handle m_handle;
	SyncEvent *m_exitEvent;
	ulong_t m_id;

	struct AsyncNotify {
		INotifyHandler *handler;
		int index;
	};
	std::list<AsyncNotify> m_asyncNotifyList;
};



AX_END_NAMESPACE

#endif // AX_THREAD_H
