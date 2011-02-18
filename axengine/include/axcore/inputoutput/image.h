/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_ENGINE_IMAGE_H
#define AX_ENGINE_IMAGE_H

AX_BEGIN_NAMESPACE

struct TexType
{
	enum Type {
		Unknown,
		_2D,
		_3D,
		CUBE,
		MaxType
	};
	AX_DECLARE_ENUM(TexType);
};

struct CubeMapFace
{
	enum Type {
		PX, NX, PY, NY, PZ, NZ,

		MaxType
	};
	AX_DECLARE_ENUM(CubeMapFace);
};

struct TexFormat
{
	enum Type {
		AUTO,			// auto specify format for file data

		NULLTARGET, R5G6B5,	// for directx shadow map null color target

		RGB10A2,		// just for render target

		RG16,			// for indirect texture

		// 8 bits image
		L8, LA8, A8, BGR8, BGRA8, BGRX8,

		// compressed image
		DXT1, DXT3, DXT5,

		// 16 bits fixed image
		L16,				// 16 bits int texture, terrain heightmap use this

		// 16 bits float image
		R16F, RG16F, RGB16F, RGBA16F,

		// 32 bits float image
		R32F, RG32F, RGB32F, RGBA32F,

		// depth texture
		D16, D24, D32F,

		// depth-stencil
		D24S8,

		// vendor specified
		DF16, DF24, RAWZ, INTZ,

		MAX_NUMBER
	};
	AX_DECLARE_ENUM(TexFormat);

	bool isByte() const;
	bool isUShort() const;
	bool isDXTC() const;
	bool isFloat() const;
	bool isHalf() const;
	bool isColor() const { return m_t >= L8 && m_t <= RGBA32F; }
	bool isDepth() const { return m_t >= D16 && m_t <= INTZ; }
	bool isStencil() const { return m_t == D24S8; }
	bool isCompressed() const;
	int getNumComponents() const;
	int getBitsPerPixel() const;
	int calculateDataSize(int width, int height) const;
	int getBlockSize() const;
	int getBlockDataSize() const;
	int getDepthBits() const;
	const char *toString() const;
};

inline bool TexFormat::isByte() const
{
	return m_t >= L8 && m_t <= BGRX8;
}

inline bool TexFormat::isUShort() const
{
	return m_t == L16 || m_t == RG16;
}

inline bool TexFormat::isDXTC() const
{
	return ((m_t == DXT1) || (m_t == DXT3) || (m_t == DXT5));
}

inline bool TexFormat::isFloat() const
{
	return m_t >= R32F && m_t <= RGBA32F;
}

inline bool TexFormat::isHalf() const
{
	return m_t >= R16F && m_t <= RGBA16F;
}

inline bool TexFormat::isCompressed() const
{
	return isDXTC();
}

inline int TexFormat::getNumComponents() const
{
	switch (m_t) {
	case RG16:
		return 2;
	case L8:				// 8 bits luminance: alpha is always 1.0, Uint8
		return 1;
	case LA8:				// 8 bits luminance & alpha
		return 2;
	case A8:				// 8 bits alpha: font texture use this
		return 1;
	case BGR8:
		return 3;
	case BGRA8:
		return 4;
	case BGRX8:
		return 4;

	case DXT1:
		return 3;
	case DXT3:
	case DXT5:
		return 4;

	case L16:
		return 1;					// 16 bits short int, for terrain or other use, UShort

	// 16 bits float image
	case R16F:			// 16 bits float luminance: alpha is always 1.0
		return 1;
	case RG16F:
		return 2;
	case RGB16F:
		return 3;
	case RGBA16F:
		return 4;

	// 32 bits float image
	case R32F:			// 32 bits float luminance: alpha is always 1.0
		return 1;
	case RG32F:
		return 2;
	case RGB32F:
		return 3;
	case RGBA32F:
		return 4;

	case D16:
	case D24:
	case D32F:
	case D24S8:
	case DF16:
	case DF24:
	case RAWZ:
	case INTZ:
		return 1;

	default:
		Errorf(_("getNumComponents: unknown type"));
		return 0;
	};
}

// get image's bits per pixel
inline int TexFormat::getBitsPerPixel() const
{
	if (isDXTC()) {
		switch (m_t) {
		case DXT1:
			return 4;
		case DXT3:
		case DXT5:
			return 8;
		}
	}

	if (isByte())
		return getNumComponents() * 8;

	if (isUShort() || isHalf())
		return getNumComponents() * 16;

	if (isFloat())
		return getNumComponents() * 32;

	if (m_t == NULLTARGET) return 0;
	if (m_t == R5G6B5) return 16;
	if (m_t == RGB10A2) return 32;
	if (m_t == D16 || m_t == DF16) return 16;
	if (m_t == D24) return 24;
	if (m_t == D32F) return 32;
	if (m_t == D24S8 || m_t == DF24 || m_t == RAWZ || m_t == INTZ) return 32;

	Errorf(_("getBitsPerPixel: unknown type"));

	return 0;
}


// return number bytes
inline int TexFormat::calculateDataSize(int width, int height) const
{

	if (isDXTC()) {
		switch (m_t) {
		case DXT1:
			return ((width+3)/4)*((height+3)/4)* 8;		// dxt1's block size is 8
		case DXT3:
		case DXT5:
			return ((width+3)/4)*((height+3)/4)* 16;	// dxt3,dxt5's block size is 16
		}
	}

	int num_pixels = width * height;
	return num_pixels * getBitsPerPixel() >> 3;
}

inline int TexFormat::getBlockSize() const
{
	if (isDXTC()) {
		return 4;
	} else {
		return 1;
	}
}

inline int TexFormat::getBlockDataSize() const
{
	if (isDXTC()) {
		switch (m_t) {
		case DXT1:
			return 8;
		case DXT3:
		case DXT5:
			return 16;	// dxt3,dxt5's block size is 16
		}
	}

	return getBitsPerPixel() >> 3;
}

inline int TexFormat::getDepthBits() const
{
	if (!isDepth()) {
		return 0;
	}

	switch (m_t) {
	case D16:
	case DF16:
		return 16;
	case D24:
	case D24S8:
	case DF24:
	case RAWZ:
	case INTZ:
		return 24;
	case D32F:
		return 32;
	default:
		return 0;
	}
}

inline const char *TexFormat::toString() const
{
	switch (m_t) {
	case NULLTARGET:
		return "NULL";
	case R5G6B5:
		return "R5G6B5";
	case RGB10A2:
		return "RGB10A2";
	case RG16:
		return "RG16";
	case L8:				// 8 bits luminance: alpha is always 1.0, Uint8
		return "L8";
	case LA8:				// 8 bits luminance & alpha
		return "LA8";
	case A8:				// 8 bits alpha: font texture use this
		return "A8";
	case BGR8:
		return "BGR8";
	case BGRA8:
		return "BGRA8";
	case BGRX8:
		return "BGRX8";

	case DXT1:
		return "DXT1";
	case DXT3:
		return "DXT3";
	case DXT5:
		return "DXT5";

	case L16:
		return "L16";					// 16 bits short int, for terrain or other use, UShort

		// 16 bits float image
	case R16F:			// 16 bits float luminance: alpha is always 1.0
		return "R16F";
	case RG16F:
		return "RG16F";
	case RGB16F:
		return "RGB16F";
	case RGBA16F:
		return "RGBA16F";

		// 32 bits float image
	case R32F:			// 32 bits float luminance: alpha is always 1.0
		return "R32F";
	case RG32F:
		return "RG32F";
	case RGB32F:
		return "RGB32F";
	case RGBA32F:
		return "RGBA32F";

	case D16:
		return "D16";
	case D24:
		return "D24";
	case D32F:
		return "D32F";
	case D24S8:
		return "D24S8";

	case DF16:
		return "DF16";
	case DF24:
		return "DF24";
	case RAWZ:
		return "RAWZ";
	case INTZ:
		return "INTZ";

	default:
		Errorf(_("toString: unknown type"));
		return "NONAME";
	};
}


class AX_API Image
{
public:

	enum LoadFlag {
		NoCompressed = 2,		// don't load compressed file
		ExpandAlpha = 4,		// expand alpha, fill alpha if need
		Mipmap = 0x40,		// generate mipmap if need
		Cubemap = 0x80,		// is a cubemap
	};

	enum FileType {
		NONE, DDS, TGA, HDR, JP2,
	};

	typedef shared_array<byte_t>	DataBuffer;
	typedef std::vector<DataBuffer>	DataBufferSeq;

	Image();
	virtual ~Image();

	// io & processing
	bool loadFile(const std::string &filename, int flags = 0);
	bool loadFileByType(const std::string &filename, FileType filetype, int flags = 0);
	bool loadFileFromMemory(size_t filesize, void *filedata, FileType filetype);

	Image *resize(int width, int height);
	Image *getFloatHeightmap(float max_height);
	void generateMipmaps();
	void generateNormalmapMipmaps();
	void generateHeightmapMipmaps();	// mipmapping for height map, don't filter
	void initImage(TexFormat format, int width, int height, const byte_t *pixel = NULL);
	void initImage(TexFormat format, int width, int height, bool mipmap);
	void clear();		// clear data

	void convertFormatTo(TexFormat format);

	// set single pixel on one pos or rect, variant pixel data corresponding to Image::Format
	void setPixel(int level, const Point &pos, const byte_t *pixel);
	void setPixel(int level, int x, int y, const byte_t *pixel);
	void setPixel(int level, const Rect &rect, const byte_t *pixel);
	void setPixel(int level, int x, int y, int width, int height, const byte_t *pixel);

	// level = 0 if level don't provided
	void setPixel(const Point &pos, const byte_t *pixel);
	void setPixel(int x, int y, const byte_t *pixel);
	void setPixel(const Rect &rect, const byte_t *pixel);
	void setPixel(int x, int y, int width, int height, const byte_t *pixel);

	// get single pixel, returned value corresponding to Image::Format
	const byte_t *getPixel(int level, const Point &pos) const;
	const byte_t *getPixel(int level, int x, int y) const;
	ByteSeq getFilteredPixel(int level, const Vector2 &st) const;
	ByteSeq getFilteredPixel(int level, float s, float t) const;

	// get subimage, if image is compressed format like DXTn, rect must be block aligned, or will raise an error
	Image *readSubImage(int level, const Rect &rect) const;

	// copy block data from another image, source image and dst image must have same format
	void writeSubImage(const Image *src_image, int src_level, const Rect &src_rect, int dst_level, const Point &dst_pos);
	void writeSubImage(const Image *src_image, const Rect &src_rect, const Point &dst_pos);
	void writeSubImage(const Image *src_image, int src_level, const Rect &src_rect, int dst_level, const Rect &dst_rect, bool need_filter = false);
	void writeSubImage(const Image *src_image, const Rect &src_rect, const Rect &dst_rect, bool need_filter = false);

	// get info
	// mipmap level width, height, depth for non_power_of_two texture use same floor convention which identical to D3D and OpenGL API
	int getWidth(int level = 0) const;
	int getHeight(int level = 0) const;
	int getNumMipmapLevels() const;
	TexFormat getFormat() const;
	std::string getRealName() const;

	// get data
	const byte_t *getData(int level) const;
	const byte_t *getData(int level, TexFormat format);
	void writeDataTo(int level, byte_t *data) const;
	void writeDataTo(int level, TexFormat format, byte_t *data) const;
	byte_t *getWritablePointer(int level = 0);
	int getTotalDataSize() const;
	int getDataSize(int level) const;

	// set data
	void setData(int level, const void *data, int size);

	// rotate must be one of 0, 90, 180, 270, it is clock-wise rotate
	void writeDataTo(int level, const Rect &rect, int rotate, byte_t *data, bool flipS = false, bool flipT = false) const;

	static void writeTGA(const std::string &filename, const byte_t *pixels, int width, int height);

	// load function
	bool loadFile_tga(const std::string &filename);
	bool loadFile_hdr(const std::string &filename);
	bool loadFile_dds(const std::string &filename);
	bool loadFileFromMemory_dds(size_t filesize, void *filedata);

	// save function
	void saveFile_dds(const std::string &filename, bool bFast = true);
	void saveFile_tga(const std::string &filename);

	bool loadFile_raw(const std::string &filename, int width, int height);
	void saveFile_raw(const std::string &filename);

protected:
	int convertFormatTo(TexFormat format, int level, byte_t *pBuf) const;

private:
	TexFormat m_format;
	bool m_dataPresent;
	int m_loadFlags;

	std::string m_realName;
	int m_width, m_height, m_depth;
	DataBufferSeq m_datas;
};

AX_END_NAMESPACE

#endif // AX_ENGINE_IMAGE_H
