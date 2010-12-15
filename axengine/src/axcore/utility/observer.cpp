/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "../private.h"

AX_BEGIN_NAMESPACE

IObservable::~IObservable() {}

void IObservable::addObserver(IObserver *observer) {
	m_observers.push_back(observer);
}

void IObservable::removeObserver(IObserver *observer) {
#if 0
	std::list<IObserver*>::iterator it = m_observers.begin();

	for (; it != m_observers.end(); ++it) {
		if (*it == observer) {
			m_observers.erase(it);
			return;
		}
	}

	Errorf("Observable::detachObserver: cann't find observer to detach");
#else
	m_observers.remove(observer);
#endif
}

void IObservable::notifyObservers(int arg) {
	std::list<IObserver*>::iterator it = m_observers.begin();

	for (; it != m_observers.end(); ++it) {
		(*it)->beNotified(this, arg);
	}
}

AX_END_NAMESPACE
