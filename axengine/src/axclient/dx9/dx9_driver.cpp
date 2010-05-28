/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "dx9_private.h"


AX_BEGIN_NAMESPACE

AX_IMPLEMENT_FACTORY(DX9_Driver)

// console command
AX_BEGIN_COMMAND_MAP(DX9_Driver)
AX_END_COMMAND_MAP()

DX9_Driver::DX9_Driver()
{
	m_initialized = false;
}

DX9_Driver::~DX9_Driver()
{}

void DX9_Driver::initialize()
{
	if (m_initialized) {
		return;
	}

	g_renderDriver = this;
	d3d9Driver = this;

	Printf("..Initializing D3D9Driver...\n");

	d3d9InternalWindow = new DX9_Window("d3d9window");
	d3d9DriverInfo = new RenderDriverInfo;
	d3d9DriverInfo->driverType = RenderDriverInfo::D3D;
	d3d9DriverInfo->highestQualitySupport = ShaderQuality::Low;
	d3d9DriverInfo->vendor = "unknown";
	d3d9DriverInfo->renderer = "unknown";
	d3d9DriverInfo->version = "unknown";
	d3d9DriverInfo->extension = "unknown";

	Printf("...Calling Direct3DCreate9(D3D_SDK_VERSION = %d)...", D3D_SDK_VERSION);
	d3d9Api = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9Api) {
		Errorf("D3D9Driver::initialize: Direct3DCreate9 error\nMaybe you should install DirectX 9 or later version runtime");
		return;
	}

	D3DCAPS9 caps;

	HRESULT hr = d3d9Api->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	if (FAILED(hr)) {
		Errorf("D3D9Driver::initialize: GetDeviceCaps failed %s", D3DErrorString(hr));
	}

	d3d9DriverInfo->maxTextureUnits = caps.MaxTextureBlendStages;
	d3d9DriverInfo->maxTextureSize = std::min(caps.MaxTextureWidth, caps.MaxTextureHeight);
	d3d9DriverInfo->max3DTextureSize = caps.MaxVolumeExtent;
	d3d9DriverInfo->maxCubeMapTextureSize = d3d9DriverInfo->maxTextureSize;


	// Get the current desktop format
	D3DDISPLAYMODE dispmode;
	hr = d3d9Api->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dispmode);
	if (FAILED(hr)) {
		Errorf("GetAdapterDisplayMode failed %s", D3DErrorString(hr));
	}

	// create d3d9 device
	D3DPRESENT_PARAMETERS d3dpp;
	memset(&d3dpp, 0, sizeof(d3dpp));
	d3dpp.BackBufferWidth = 0;
	d3dpp.BackBufferHeight = 0;
	d3dpp.BackBufferFormat = dispmode.Format;
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil= FALSE;
	d3dpp.AutoDepthStencilFormat= D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = (HWND)d3d9InternalWindow->getHandle();
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	DWORD BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE;

	hr = d3d9Api->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,(HWND)d3d9InternalWindow->getHandle(), BehaviorFlags, &d3dpp, &d3d9Device);
	if (FAILED(hr)) {
		Errorf("CreateDevice failed %s", D3DErrorString(hr));
	}

#if 0
	d3d9StateManager = new D3D9StateManager();
	d3d9StateManager->DirtyCachedValues();

	V(d3d9StateManager->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL));

	d3d9QueryManager = new D3D9querymanager();
	g_queryManager = d3d9QueryManager;

	d3d9VertexBufferManager = new D3D9VertexBufferManager();

	d3d9PrimitiveManager = new D3D9primitivemanager();
	g_primitiveManager = d3d9PrimitiveManager;
	d3d9TargetManager = new D3D9TargetManager();
	g_targetManager = d3d9TargetManager;
#endif
	g_shaderMacro.setMacro(ShaderMacro::G_D3D);
	d3d9ShaderManager = new D3D9ShaderManager;
#if 0
	g_shaderManager = d3d9ShaderManager;

	d3d9TextureManager = new D3D9texturemanager();
	g_textureManager = d3d9TextureManager;
//#else
	D3D9Texture::initManager();
	d3d9Thread = new D3D9Thread();
	if (r_multiThread.getInteger()) {
		d3d9Thread->startThread();
	}

	d3d9Draw = new D3D9Draw();
#endif

	d3d9InternalWindow->bind();

	Printf("ok\n");
}

void DX9_Driver::finalize()
{}

#if 0
void D3D9Driver::postInit()
{
	d3d9Postprocess = new D3D9Postprocess();
}
#endif

#if 0
bool D3D9Driver::isHDRRendering()
{ return false; }

RenderTarget *D3D9Driver::createWindowTarget(Handle wndId, const String &name)
{
	DX9_Window *state = new DX9_Window(wndId, name);
	AX_ASSERT(state);
	return state;
}
#endif

const RenderDriverInfo *DX9_Driver::getDriverInfo()
{
	return d3d9DriverInfo;
}

#if 0
uint_t D3D9Driver::getBackendCaps() {
	return 0;
}
void D3D9driver::preFrame() {
	d3d9Thread->preFrame();
}

void D3D9driver::beginSelect(const RenderCamera &view) {}

void D3D9driver::loadSelectId(int id) {}

void D3D9driver::testActor(Actor *re) {}

void D3D9driver::testPrimitive(Primitive *prim) {}

SelectRecordSeq D3D9driver::endSelect() {
	return SelectRecordSeq();
}
#endif
void DX9_Driver::reset( int width, int height, bool fullscreen )
{
	onDeviceLost();
	onReset();
}

void DX9_Driver::onReset()
{
#if 0
	d3d9PrimitiveManager->onReset();
	d3d9TargetManager->onReset();
#endif
}

void DX9_Driver::onDeviceLost()
{
#if 0
	d3d9TargetManager->onDeviceLost();
	d3d9PrimitiveManager->onDeviceLost();
#endif
}

#if 0
bool D3D9Driver::isInRenderingThread()
{
	return d3d9Thread->isCurrentThread();
}

#endif

IDirect3DSurface9 * DX9_Driver::getDepthStencil(int width, int height)
{
	return 0;
}

AX_END_NAMESPACE

