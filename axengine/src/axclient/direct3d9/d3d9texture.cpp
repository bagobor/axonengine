/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "d3d9private.h"

AX_BEGIN_NAMESPACE

#if 0
static D3D9texturemanager *s_manager;

inline bool trTexFormat(TexFormat texformat, D3DFORMAT &d3dformat)
{
	d3dformat = D3DFMT_UNKNOWN;

	switch (texformat) {
	case TexFormat::NULLTARGET:
		d3dformat = (D3DFORMAT)MAKEFOURCC('N','U','L','L');
		break;

	case TexFormat::R5G6B5:
		d3dformat = D3DFMT_R5G6B5;
		break;

	case TexFormat::RGB10A2:
		d3dformat = D3DFMT_A2R10G10B10;
		break;

	case TexFormat::RG16:
		d3dformat = D3DFMT_G16R16;
		break;

	case TexFormat::L8:
		d3dformat = D3DFMT_L8;
		break;

	case TexFormat::LA8:
		d3dformat = D3DFMT_A8L8;
		break;

	case TexFormat::A8:
		d3dformat = D3DFMT_A8;
		break;

	case TexFormat::BGR8:
		d3dformat = D3DFMT_R8G8B8;
		break;

	case TexFormat::BGRA8:
		d3dformat = D3DFMT_A8R8G8B8;
		break;

	case TexFormat::BGRX8:
		d3dformat = D3DFMT_X8R8G8B8;
		break;

	case TexFormat::DXT1:
		d3dformat = D3DFMT_DXT1;
		break;

	case TexFormat::DXT3:
		d3dformat = D3DFMT_DXT3;
		break;

	case TexFormat::DXT5:
		d3dformat = D3DFMT_DXT5;
		break;

	case TexFormat::L16:
		d3dformat = D3DFMT_L16;
		break;


	// 16 bits float texture
	case TexFormat::R16F:
		d3dformat = D3DFMT_R16F;
		break;

	case TexFormat::RG16F:
		d3dformat = D3DFMT_G16R16F;
		break;

	case TexFormat::RGB16F:
//			d3dformat = GL_RGB16F;
		break;

	case TexFormat::RGBA16F:
		d3dformat = D3DFMT_A16B16G16R16F;
		break;


	// 32 bits float texture
	case TexFormat::R32F:
		d3dformat = D3DFMT_R32F;
		break;

	case TexFormat::RG32F:
		d3dformat = D3DFMT_G32R32F;
		break;

	case TexFormat::RGB32F:
//			d3dformat = D3DFMT_A32B32G32R32F;
		break;

	case TexFormat::RGBA32F:
		d3dformat = D3DFMT_A32B32G32R32F;
		break;

	case TexFormat::D16:
		d3dformat = D3DFMT_D16;
		break;

	case TexFormat::D24:
		d3dformat = D3DFMT_D24X8;
		break;

	case TexFormat::D32:
		d3dformat = D3DFMT_D32;
		break;

	case TexFormat::D24S8:
		d3dformat = D3DFMT_D24S8;
		break;

	case TexFormat::DF16:
		d3dformat = (D3DFORMAT)MAKEFOURCC('D','F','1','6');
		break;

	case TexFormat::DF24:
		d3dformat = (D3DFORMAT)MAKEFOURCC('D','F','2','4');
		break;

	case TexFormat::RAWZ:
		d3dformat = (D3DFORMAT)MAKEFOURCC('R','A','W','Z');
		break;

	case TexFormat::INTZ:
		d3dformat = (D3DFORMAT)MAKEFOURCC('I','N','T','Z');
		break;

	default:
		Errorf("trTexFormat: bad enum");
	}

	if (d3dformat == D3DFMT_UNKNOWN) {
		return false;
	}

	return true;
}

D3D9Texture::D3D9Texture()
{
	m_initialized = false;
	m_initFlags = 0;
	m_format = TexFormat::AUTO;
	m_object = 0;

	m_filterMode = FM_Linear;
	m_clampMode = CM_Repeat;
	m_borderColor = D3DCOLOR_RGBA(0,0,0,0);
	m_hardwareShadowMap = false;
	m_hardwareGenMipmap = false;

	m_videoMemoryUsed = 0;

#if 0
	g_statistic->incValue(stat_numTextures);
#else
	stat_numTextures.inc();
#endif
}

D3D9Texture::~D3D9Texture()
{
	D3D9_SCOPELOCK;

	SAFE_RELEASE(m_object);

#if 0
	g_statistic->decValue(stat_numTextures);
	g_statistic->subValue(stat_textureMemory, m_videoMemoryUsed);
#else
	stat_numTextures.dec();
	stat_textureMemory.sub(m_videoMemoryUsed);
#endif
}

bool D3D9Texture::doInit(const std::string &name, intptr_t arg)
{
	AX_ASSERT(!m_initialized);

	m_initFlags = arg;

	m_initialized = true;

	return loadFile2D(name);
}

void D3D9Texture::initialize(TexFormat format, int width, int height, InitFlags flags /*= 0 */)
{
	D3D9_SCOPELOCK;

	SAFE_RELEASE(m_object);

	HRESULT hr;

	m_format = format;
	m_width = width;
	m_height = height;
	m_initFlags = flags;

	bool mipmap = false;
	D3DFORMAT d3dformat;
	trTexFormat(m_format, d3dformat);

	if (d3dformat == D3DFMT_UNKNOWN) {
		Errorf("Direct3D don't support texture format '%s'", format.toString());
	}

	D3DPOOL d3dpool = D3DPOOL_MANAGED;
	DWORD d3dusage = 0;

	if (flags.isSet(Texture::IF_RenderTarget)) {
		d3dpool = D3DPOOL_DEFAULT;
		if (format.isDepth()) {
			d3dusage = D3DUSAGE_DEPTHSTENCIL;
		} else {
			d3dusage = D3DUSAGE_RENDERTARGET;
		}
	}

	if (flags.isSet(Texture::IF_AutoGenMipmap)) {
		d3dusage |= D3DUSAGE_AUTOGENMIPMAP;

		m_hardwareGenMipmap = checkIfSupportHardwareMipmapGeneration(d3dformat, d3dusage);

		if (!m_hardwareGenMipmap) {
			d3dusage &= ~D3DUSAGE_AUTOGENMIPMAP;
		}
	}

	m_videoMemoryUsed = m_format.calculateDataSize(width, height);

	V(dx9_device->CreateTexture(m_width, m_height, 1, d3dusage, d3dformat, d3dpool, &m_object, 0));

#if 0
	g_statistic->addValue(stat_textureMemory, m_videoMemoryUsed);
#else
	stat_textureMemory.add(m_videoMemoryUsed);
#endif

	setPrivateData();
}

void D3D9Texture::initialize( const FixedString &name, InitFlags flags )
{
	doInit(name, flags);
}

void D3D9Texture::getSize(int &width, int &height, int &depth)
{
	width = m_width;
	height = m_height;
	depth = m_depth;
}

void D3D9Texture::getSize(int &width, int &height)
{
	width = m_width;
	height = m_height;
}

void D3D9Texture::uploadSubTextureIm(const Rect &rect, const void *pixels, TexFormat format /*= TexFormat::AUTO */)
{
	D3D9_SCOPELOCK;

	if (!format) {
		format = m_format;
	}

	if (rect.isEmpty()) {
		return;
	}

	D3DFORMAT d3dformat;
	trTexFormat(format, d3dformat);

	if (d3dformat == D3DFMT_UNKNOWN) {
		return;
	}

	LPDIRECT3DSURFACE9 surface;
	HRESULT hr;
	V(m_object->GetSurfaceLevel(0, &surface));

	RECT d3drect;
	d3drect.left = rect.x; d3drect.top = rect.y; d3drect.right = rect.xMax(); d3drect.bottom = rect.yMax();

	RECT d3dsrcrect;
	d3dsrcrect.left = 0; d3dsrcrect.top = 0; d3dsrcrect.right = rect.width; d3dsrcrect.bottom = rect.height;

	UINT srcpitch = format.calculateDataSize(rect.width, 1);
	DWORD d3dfilter = D3DX_FILTER_NONE;

	V(D3DXLoadSurfaceFromMemory(surface, 0, &d3drect, pixels, d3dformat, srcpitch, 0, &d3dsrcrect, d3dfilter, 0));

	SAFE_RELEASE(surface);
}

void D3D9Texture::uploadSubTextureIm(int level, const Rect &rect, const void *pixels, TexFormat format /*= TexFormat::AUTO */)
{
	D3D9_SCOPELOCK;

	if (!format) {
		format = m_format;
	}

	if (rect.isEmpty()) {
		return;
	}

	D3DFORMAT d3dformat;
	trTexFormat(format, d3dformat);

	if (d3dformat == D3DFMT_UNKNOWN) {
		return;
	}

	LPDIRECT3DSURFACE9 surface;
	HRESULT hr;
	V(m_object->GetSurfaceLevel(level, &surface));

	RECT d3drect;
	d3drect.left = rect.x; d3drect.top = rect.y; d3drect.right = rect.xMax(); d3drect.bottom = rect.yMax();

	RECT d3dsrcrect;
	d3dsrcrect.left = 0; d3dsrcrect.top = 0; d3dsrcrect.right = rect.width; d3dsrcrect.bottom = rect.height;

	UINT srcpitch = format.calculateDataSize(rect.width, 1);
	DWORD d3dfilter = D3DX_FILTER_NONE;

	V(D3DXLoadSurfaceFromMemory(surface, 0, &d3drect, pixels, d3dformat, srcpitch, 0, &d3dsrcrect, d3dfilter, 0));

	SAFE_RELEASE(surface);
}

void D3D9Texture::setClampMode(ClampMode clampmode)
{
	m_clampMode = clampmode;
}

void D3D9Texture::setFilterMode(FilterMode filtermode)
{
	m_filterMode = filtermode;
}

void D3D9Texture::setBorderColor(const Rgba &color)
{
	m_borderColor = D3DCOLOR_RGBA(color.r, color.g, color.b, color.a);
}

void D3D9Texture::setHardwareShadowMap(bool enable)
{
	m_hardwareShadowMap = enable;
}

void D3D9Texture::saveToFile(const std::string &filename)
{
	std::string ospath = g_fileSystem->modPathToOsPath(filename);
	HRESULT hr;
	V(D3DXSaveTextureToFileW(u2w(ospath).c_str(), D3DXIFF_DDS, m_object, 0));
}

void D3D9Texture::generateMipmapIm()
{
	D3D9_SCOPELOCK;

	if (m_hardwareGenMipmap) {
		m_object->GenerateMipSubLevels();
		return;
	}

	if (!m_format.isDXTC()) {
		// currently, only support dxtc
		return;
	}

	HRESULT hr;
	D3DLOCKED_RECT lockedRect;
	V(m_object->LockRect(0, &lockedRect, 0, 0));
	V(m_object->UnlockRect(0));

	D3D9Texture *dummy = new D3D9Texture();
	dummy->initialize(TexFormat::BGRA8, m_width, m_height, Texture::IF_AutoGenMipmap);
	dummy->uploadSubTextureIm(Rect(0,0,m_width,m_height), lockedRect.pBits, m_format);
//		dummy->saveToFile("test.dds");

	V(dummy->m_object->LockRect(0, &lockedRect, 0, 0));
	Image image;
	image.initImage(TexFormat::BGRA8, m_width, m_height);
	image.setData(0, lockedRect.pBits, image.getDataSize(0));
	V(dummy->m_object->UnlockRect(0));

	image.generateMipmaps();

	int width = m_width;
	int height = m_height;
	for (DWORD i = 0; i < m_object->GetLevelCount(); i++) {
		if (i >= (DWORD)image.getNumMipmapLevels()) {
			break;
		}
		uploadSubTextureIm(i, Rect(0,0,width,height), image.getData(i), dummy->getFormat());

		width >>= 1;
		height >>= 1;
		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}

	delete dummy;
}

TexFormat D3D9Texture::getFormat()
{
	return m_format;
}

bool D3D9Texture::loadFile2D(const std::string &filename)
{
	D3D9_SCOPELOCK;

	HRESULT hr;
	const byte_t *data;
	int flags = 0;

	if (!(m_initFlags.isSet(IF_NoMipmap)))
		flags |= Image::Mipmap;

	flags |= Image::ExpandAlpha;

	std::auto_ptr<Image> imagefile(new Image);
	if (!imagefile->loadFile(filename, flags)) {
//			Debugf("D3D9new TextureFile2D: can't find image file for %s\n", filename.c_str());
		return false;
	}

	m_width = imagefile->getWidth();
	m_height = imagefile->getHeight();
//		mDesc.format = imagefile->getFormat();
	if (!Math::isPowerOfTwo(m_width) || !Math::isPowerOfTwo(m_height)) {
//		if (!(mDesc.flags & TexFlag_allowNPOT))
		Errorf("GLnew TextureFile2D: texture %s size isn't power of two", filename.c_str());
//	else
//		Debugf("GLnew TextureFile2D: texture %s size isn't power of two\n", mDesc.name.c_str());
	}

	m_format = imagefile->getFormat();

	D3DFORMAT d3dformat;

	trTexFormat(imagefile->getFormat(), d3dformat);

	DWORD d3dusage = 0;

	int mipdown = image_mip.getInteger();
	if (m_initFlags.isSet(IF_NoMipmap) || imagefile->getNumMipmapLevels() <= 1) {
		m_isMipmaped = false;
		mipdown = 0;
	} else {
		m_isMipmaped = true;
		mipdown = Math::clamp(mipdown, 0, imagefile->getNumMipmapLevels()-1);

		m_width >>= mipdown;
		m_height >>= mipdown;
		if (m_width < 1) m_width = 1;
		if (m_height < 1) m_height = 1;
	}


//	m_initFlags = 0;

	if (m_initFlags.isSet(Texture::IF_RenderTarget)) {
		Errorf("Can't load render target from a file");
	}

	if (m_initFlags.isSet(Texture::IF_AutoGenMipmap)) {
		d3dusage |= D3DUSAGE_AUTOGENMIPMAP;
		m_isMipmaped = true;
		m_hardwareGenMipmap = checkIfSupportHardwareMipmapGeneration(d3dformat, d3dusage);

		if (!m_hardwareGenMipmap) {
			d3dusage &= ~D3DUSAGE_AUTOGENMIPMAP;
		}
	}

	if (m_object == 0) {
		V(dx9_device->CreateTexture(m_width, m_height, !m_isMipmaped, d3dusage, d3dformat, D3DPOOL_MANAGED, &m_object, 0));
	}

	int width, height;
	width = m_width;
	height = m_height;
	for (DWORD i = 0; i < m_object->GetLevelCount(); i++) {
		if (i + mipdown >= (DWORD)imagefile->getNumMipmapLevels()) {
			// if no image for this level, break
			break;
		}

		uint_t datasize = imagefile->getFormat().calculateDataSize(width, height);
		m_videoMemoryUsed += datasize;

		data = imagefile->getData(i + mipdown);
#if 0
		LPDIRECT3DSURFACE9 surface;

		V(m_object->GetSurfaceLevel(i, &surface));

		D3DXLoadSurfaceFromMemory(surface, 0, 0, data, d3dformat, pitch, 0, rect, D3DX_FILTER_NONE, 0);
#else
		D3DLOCKED_RECT lockedRect;
		V(m_object->LockRect(i, &lockedRect, 0, 0));
		int mypitch = m_format.calculateDataSize(width, 1);
		if (mypitch != lockedRect.Pitch) {
			Errorf("mypitch != lockedRect.Pitch");
		}
		memcpy(lockedRect.pBits, data, datasize);
		V(m_object->UnlockRect(i));
#endif
		width >>= 1;
		height >>= 1;
		if (width < 1) width = 1;
		if (height < 1) height = 1;

		if (m_initFlags.isSet(Texture::IF_AutoGenMipmap)) {
//			break;
		}
	}

	if (m_initFlags.isSet(Texture::IF_AutoGenMipmap)) {
		generateMipmapIm();
	}

	if (m_isMipmaped) {
		setFilterMode(FM_Trilinear);
	} else {
		setFilterMode(FM_Linear);
	}

	setClampMode(CM_Repeat);

	setPrivateData();

#if 0
	g_statistic->addValue(stat_textureMemory, m_videoMemoryUsed);
#else
	stat_textureMemory.add(m_videoMemoryUsed);
#endif
	return true;
}

void D3D9Texture::setPrivateData()
{
#if 0
	void *data = this;
	m_object->SetPrivateData(d3d9ResGuid, &data, sizeof(void*), 0);
#else
	s_manager->addToDict(m_object, this);
#endif
}

void D3D9Texture::copyFramebuffer(const Rect &r)
{

}

void D3D9Texture::issueSamplerState(DWORD dwStage)
{
	if (m_format == TexFormat::RGBA16F) {
//			Printf("justfortest");
	}

	d3d9StateManager->setSamplerStateBlock(dwStage, m_clampMode, m_filterMode);
}

bool D3D9Texture::checkIfSupportHardwareMipmapGeneration(D3DFORMAT d3dformat, DWORD d3dusage)
{
	if (dx9_api->CheckDeviceFormat(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		d3dusage | D3DUSAGE_AUTOGENMIPMAP,
		D3DRTYPE_TEXTURE,
		d3dformat) == S_OK)
	{
		return true;
	}

	return false;
}

void D3D9Texture::initManager()
{
	s_manager = new D3D9texturemanager();
}

void D3D9Texture::finalizeManager()
{
	SafeDelete(s_manager);
}

D3D9Texture *D3D9Texture::getAppTexture( LPDIRECT3DBASETEXTURE9 d3dtex )
{
	return s_manager->getTex(d3dtex);
}

void D3D9Texture::syncFrame()
{
	s_manager->syncFrame();
}

// console command
AX_BEGIN_COMMAND_MAP(D3D9texturemanager)
	AX_COMMAND_ENTRY("dumpTex",	dumpTex_f)
AX_END_COMMAND_MAP()

static const std::string getImageFilename(const char *ext) {
	std::string filename;
	char *pattern;
	int i;

	/* find a file name */
	pattern = "images/img%i%i%i%i.%s";
	for (i = 0; i<=9999; i++) {
		StringUtil::sprintf(filename, pattern, i/1000, (i / 100) % 10, (i / 10) % 10, i%10, ext);
		if (g_fileSystem->readFile(filename, NULL) <= 0)
			break;
	}
	if (i==10000) {
		Printf("getImageFilename: Couldn't create a file\n");
		return std::string();
	}

	return filename;
}

D3D9texturemanager::D3D9texturemanager()
{
	g_cmdSystem->registerHandler(this);
}

D3D9texturemanager::~D3D9texturemanager()
{
	g_cmdSystem->removeHandler(this);
}

void D3D9texturemanager::dumpTex_f(const CmdArgs &params)
{
	if (params.tokened.size() != 2) {
		Printf("Usage: dumpTex <tex_name>\n");
		return;
	}
	const std::string &texname = params.tokened[1];

	Texture *tex = new Texture(texname);

	if (!tex) {
		Printf("Cann't found texture '%s'\n", texname.c_str());
		return;
	}

	std::string filename = getImageFilename("dds");

	tex->saveToFile(filename);
}

Texture *D3D9texturemanager::createObject()
{
	return new D3D9Texture();
}

void D3D9texturemanager::syncFrame()
{
	{
		// load commands
		LoadCmdList::const_iterator it = m_loadCmdList.begin();
		for (; it != m_loadCmdList.end(); ++it) {
			const LoadCmd *cmd = &*it;

			if (!cmd->texName.empty()) {
				cmd->texture->initialize(cmd->texName, cmd->initFlags);
			} else {
				cmd->texture->initialize(cmd->format, cmd->width, cmd->height, cmd->initFlags);
			}
		}

		m_loadCmdList.clear();
	}

	{
		// upload commands
		UploadCmdList::const_iterator it = m_uploadCmdList.begin();
		for (; it != m_uploadCmdList.end(); ++it) {
			const UploadCmd *cmd = &*it;

			if (cmd->pixel) {
				cmd->texture->uploadSubTextureIm(cmd->rect, cmd->pixel, cmd->format);
			} else {

			}
		}

		m_uploadCmdList.clear();
	}

	{
		// gen mipmap
		MipmapList::iterator it = m_needGenMipmapHead.begin();
		for (; it != m_needGenMipmapHead.end(); ++it) {
			it->generateMipmapIm();
		}
		m_needGenMipmapHead.clear();
	}

	{
		// check need free
		NeedfreeList::iterator it = m_needFreeHead.begin();
		while (it != m_needFreeHead.end()) {
			D3D9Texture *owner = (D3D9Texture *)&*it;
			NeedfreeList::iterator next = ++it;
			m_needFreeHead.erase(owner);
			m_textureDict.erase(owner->getKey());
			SafeDelete(owner);

			it = next;
		}
		m_needFreeHead.clear();
	}
}

#endif

AX_END_NAMESPACE
