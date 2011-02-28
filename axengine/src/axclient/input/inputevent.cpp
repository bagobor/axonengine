/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "../private.h"

#include "wininput.h"

AX_BEGIN_NAMESPACE

std::string InputKey::getName() const {
	if (m_t < 32) {
		Errorf("Error InputKey");
	}

	if (m_t >= 32 && m_t < 128) {
		return std::string(1,char(m_t));
	}

#define KEYITEM(key) case key: return #key; break;

	switch (m_t) {
		AX_KEYITEMS
	}
#undef KEYITEM

	return std::string();
}

InputKey::Type InputKey::getKey(const std::string &keyname)
{
	if (keyname.empty()) {
		Errorf("error keyname");
	}

	if (keyname.size() == 1) {
		return (InputKey::Type)keyname[0];
	}

	static Dict<std::string,InputKey> nameDict;
	if (nameDict.empty()) {
#define KEYITEM(key) nameDict[#key] = key;
		AX_KEYITEMS
#undef KEYITEM
	}

	Dict<std::string,InputKey>::const_iterator it = nameDict.find(keyname);
	if (it == nameDict.end()) {
		return InputKey::Invalid;
	}

	return it->second;
}

//--------------------------------------------------------------------------
// class IInputHandler
//--------------------------------------------------------------------------

void IInputHandler::handleEvent(InputEvent *e) {
	preHandleEvent(e);

	onEvent(e);

	if (e->accepted)
		goto exit;

	switch (e->type) {
	case InputEvent::KeyDown:
		{
			InputEvent *ke = static_cast<InputEvent*>(e);
			onKeyDown(ke);

			break;
		}

	case InputEvent::KeyUp:
		{
			InputEvent *ke = static_cast<InputEvent*>(e);
			onKeyUp(ke);

			break;
		}
	case InputEvent::MouseDown:
		{
			InputEvent *me = static_cast<InputEvent*>(e);
			onMouseDown(me);

			break;
		}

	case InputEvent::MouseUp:
		{
			InputEvent *me = static_cast<InputEvent*>(e);
			onMouseUp(me);

			break;
		}

	case InputEvent::MouseMove:
		{
			InputEvent *me = static_cast<InputEvent*>(e);
			onMouseMove(me);

			break;
		}

	case InputEvent::Wheel:
		{
			InputEvent *we = static_cast<InputEvent*>(e);
			onMouseWheel(we);

			break;
		}
	}

exit:
	postHandleEvent(e);
}

void IInputHandler::onEvent(InputEvent *e) {}
void IInputHandler::onKeyDown(InputEvent *e) {}
void IInputHandler::onKeyUp(InputEvent *e) {}
void IInputHandler::onMouseDown(InputEvent *e) {}
void IInputHandler::onMouseUp(InputEvent *e) {}
void IInputHandler::onMouseMove(InputEvent *e) {}
void IInputHandler::onMouseWheel(InputEvent *e) {}


//--------------------------------------------------------------------------
// class InputSystem
//--------------------------------------------------------------------------

InputSystem::InputSystem() {
	m_eventReadPos = 0;
	m_eventWritePos = 0;
	m_gameWnd = 0;
	m_isCapturing = false;

	g_coreSystem->registerTickable(CoreSystem::TickEvent, this);
}

InputSystem::~InputSystem() {
	g_coreSystem->removeTickable(CoreSystem::TickEvent, this);
}

void InputSystem::initialize() {
	m_winInput = new WinInput;
}

void InputSystem::finalize() {
	SafeDelete(m_winInput);
}

void InputSystem::tick()
{
	if (!m_isCapturing) {
		return;
	}

	processEvents();
}

void InputSystem::queEvent(const InputEvent &e) {
	if (!m_isCapturing) {
		return;
	}

	if (m_eventWritePos - m_eventReadPos > EVENT_POOL_SIZE) {
		Debugf("%s: event overflowed.\n", __func__);
		return;
	}

	m_events[m_eventWritePos&(EVENT_POOL_SIZE-1)] = e;
	m_eventWritePos++;
}

InputEvent *InputSystem::getEvent() {
	if (m_eventReadPos == m_eventWritePos) {
		return nullptr;
	}

	return &m_events[m_eventReadPos++ & (EVENT_POOL_SIZE-1)];
}

void InputSystem::clearEvents() {
	m_eventReadPos = m_eventWritePos;
}

void InputSystem::startCapture(CaptureMode mode) {
	m_isCapturing = true;
	m_captureMode = mode;

	std::list<IInputSource*>::iterator it = m_eventSources.begin();

	m_winInput->setWindow(m_gameWnd);
	m_winInput->startCapture(mode);

	for (; it != m_eventSources.end(); ++it) {
		(*it)->startCapture(mode);
	}
}

void InputSystem::processEvents() {
	std::list<IInputSource*>::iterator it = m_eventSources.begin();

	m_winInput->process();

	for (; it != m_eventSources.end(); ++it) {
		(*it)->process();
	}
}

void InputSystem::setVibration(float left, float right) {
	std::list<IInputSource*>::iterator it = m_eventSources.begin();

	for (; it != m_eventSources.end(); ++it) {
		(*it)->setVibration(left, right);
	}
}

void InputSystem::setMouseMode(MouseMode mode)
{
	std::list<IInputSource*>::iterator it = m_eventSources.begin();

	for (; it != m_eventSources.end(); ++it) {
		(*it)->setMouseMode(mode);
	}
}

void InputSystem::stopCapture() {
	std::list<IInputSource*>::iterator it = m_eventSources.begin();

	for (; it != m_eventSources.end(); ++it) {
		(*it)->stopCapture();
	}

	m_winInput->stopCapture();

	m_isCapturing = false;
}

void InputSystem::registerEventSource(IInputSource *eventsource) {
	m_eventSources.push_back(eventsource);
}

void InputSystem::removeEventSource(IInputSource *eventsource) {
	m_eventSources.remove(eventsource);
}

void InputSystem::setGameWindow(RenderTarget *gamewindow)
{
	m_gameWnd = gamewindow;
}

void InputSystem::queWinInput(void *msg)
{
	m_winInput->queWinInput((MSG*)msg);
}

AX_END_NAMESPACE

