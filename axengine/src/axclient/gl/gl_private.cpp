#include "gl_private.h"

AX_BEGIN_NAMESPACE

AX_BEGIN_CLASS_MAP(AxGL)
	AX_CLASS_ENTRY("Driver", GL_Driver)
AX_END_CLASS_MAP()

GL_Driver *gl_driver;
GL_Window *gl_window;
HGLRC gl_context; // OpenGL Rendering Context

AX_END_NAMESPACE
