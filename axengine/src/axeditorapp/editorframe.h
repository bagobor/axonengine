/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef EDITORFRAME_H
#define EDITORFRAME_H

#include "private.h"
#include "frame.h"

class EditorFrame : public QWidget, public IPanel, public IViewFrame, public IInputSource
{
	Q_OBJECT

public:
    EditorFrame(QWidget *parent, View *view);
    ~EditorFrame();

	// public function
	View *getView() { return m_editorView; }

	// implement IViewFrame
	virtual RenderTarget *renderTarget();
	virtual Size size();
	virtual void setCursor(CursorType cursor_type);
	virtual void resetCursor();
	virtual void setCursorPos(const Point &pos);
	virtual void setNeedUpdate();
	virtual void setAutoUpdate(bool b);
	virtual void registerEventSource();
	virtual void removeEventSource();

	// implement IPanel
	virtual QString getTitle() { return u2q(m_editorView->getTitle()); }
	virtual QWidget *getWidget() { return this; }

	// implement IInputSource
	virtual void startCapture(InputSystem::CaptureMode capturemode);
	virtual void process();
	virtual void setVibration(float left, float right);
	virtual void setMouseMode(InputSystem::MouseMode mode);
	virtual void stopCapture();

	// overload
	virtual bool eventFilter(QObject * watched, QEvent * event);

	// no paint engine
	QPaintEngine *paintEngine() const { return NULL; }


protected:
	void translateMouseEvent(QMouseEvent *e, InputEvent *xe);
	// implement QT event handler
	virtual void paintEvent(QPaintEvent *pe);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void wheelEvent(QWheelEvent *e);
	virtual void enterEvent(QEvent *e);
	virtual void leaveEvent(QEvent *e);
#if 0
	virtual void tabletEvent(QTabletEvent *e);
#endif
	virtual void keyPressEvent(QKeyEvent * e);
	virtual void keyReleaseEvent(QKeyEvent *e);
	virtual void timerEvent(QTimerEvent *e);

#if 0
	virtual bool winEvent (MSG * message, long * result);
#endif

	void issueEvent(QEvent *qe, InputEvent &xe);

private:
    View *m_editorView;
	RenderTarget *m_renderTarget;
	QHash<int,int> m_keymap;
	int m_autoUpdateTimer;
	bool m_isCapturing;
	bool m_tabletPressed;
	InputSystem::CaptureMode m_captureMode;
	InputSystem::MouseMode m_mouseMode;
	Point m_mouseCenter;
};

#endif // EDITORFRAME_H
