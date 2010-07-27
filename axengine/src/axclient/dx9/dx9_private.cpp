/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "dx9_private.h"

AX_BEGIN_NAMESPACE

AX_BEGIN_CLASS_MAP(AxDX9)
	AX_CLASS_ENTRY("Driver", DX9_Driver)
AX_END_CLASS_MAP()

DX9_Window *dx9_internalWindow;
DX9_Driver *dx9_driver;
IDirect3D9 *dx9_api;
IDirect3DDevice9 *dx9_device;
DX9_ShaderManager *dx9_shaderManager;
ConstBuffers *dx9_constBuffers;



#if 0
RenderQueue *d3d9Queue;
SyncMutex d3d9Mutex;

D3D9TargetManager *d3d9TargetManager;
D3D9querymanager *d3d9QueryManager;
D3D9primitivemanager *d3d9PrimitiveManager;
D3D9Thread *d3d9Thread;
D3D9VertexBufferManager *d3d9VertexBufferManager;
D3D9Postprocess *d3d9Postprocess;
D3D9StateManager *d3d9StateManager;
D3D9Draw *d3d9Draw;
// {92F6401F-2E38-4ac6-8F10-3B28A89079EA}
const GUID d3d9ResGuid = { 0x92f6401f, 0x2e38, 0x4ac6, { 0x8f, 0x10, 0x3b, 0x28, 0xa8, 0x90, 0x79, 0xea } };

bool d3d9NULL;
bool d3d9NVDB;

RenderCamera *d3d9Camera;
#endif

AX_END_NAMESPACE

