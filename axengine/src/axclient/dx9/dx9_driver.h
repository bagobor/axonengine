/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_D3D9DRIVER_H
#define AX_D3D9DRIVER_H

AX_BEGIN_NAMESPACE

class DX9_Driver : public IRenderDriver, public ICmdHandler
{
public:
	AX_DECLARE_FACTORY();
	AX_DECLARE_COMMAND_HANDLER(DX9_Driver);

	DX9_Driver();
	virtual ~DX9_Driver();

	// implement IRenderDriver
	virtual void initialize();
	virtual void finalize();

	virtual const ShaderInfo *findShaderInfo(const FixedString &key);


	void reset(int width, int height, bool fullscreen);
	void onReset();
	void onDeviceLost();

	IDirect3DSurface9 *getDepthStencil(const Size &size);

protected:
	bool checkTextureFormatSupport(TexFormat format, D3DFORMAT d3dformat);
	bool checkHardwareMipmapGenerationSupport(TexFormat format, D3DFORMAT d3dformat);
	bool checkRenderTargetFormatSupport(TexFormat format, D3DFORMAT d3dformat);
	void checkFormats();

private:
	bool m_initialized;
};

AX_END_NAMESPACE


#endif // end guardian

