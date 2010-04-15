
#include "private.h"

AX_BEGIN_NAMESPACE

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Handle GLdriver::createGLWindow(const String &wnd_name)
{
	WString ws = u2w(wnd_name);
	WNDCLASSEXW wcex;	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)WHITE_BRUSH;
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = ws.c_str();
	wcex.hIconSm = 0;

	if (!::RegisterClassExW(&wcex)) {
		Errorf("GLdriver::CreateGLWindow: cann't create OpenGL window");
		return Handle(0);
	}

	DWORD dwStyle,dwExStyle;
	dwExStyle = 0;		
	dwStyle = WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;

	HWND hwnd = ::CreateWindowExW(dwExStyle
		, ws.c_str()
		, ws.c_str()
		, dwStyle
		, 0, 0, 640,480
		, NULL, NULL
		, (HINSTANCE)GetModuleHandle(NULL)
		, NULL);

	return Handle(hwnd);
}

AX_END_NAMESPACE


