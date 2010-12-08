/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#ifndef AX_RENDER_TEXTUREATLAS_H
#define AX_RENDER_TEXTUREATLAS_H

AX_BEGIN_NAMESPACE

/*!
	\class TextureAtlas

	TextureAtlas for small same size texture pieces management like font textures
	and terrain alpha textures.
*/
class AX_API TextureAtlas {
public:
	TextureAtlas();
	~TextureAtlas();

	void initialize(const std::string &name, int chunk_width, int chunk_height, int side_length, TexFormat format);
	void finalize();
	uint_t newFrame();
	bool isChunkResident(int id);
	// if texture pool is full, return false
	bool updateChunk(int id, byte_t *data, TexFormat format = TexFormat::AUTO);
	void getChunkInfo(int id, Texture*& tex, Vector4 &tc);

	void setTextureFilterMode(SamplerStateDesc::FilterMode mode);

	enum { MAX_TEXTURES = 8 };
	static const int INVALID_Chunk = -1;

protected:
	void getChunkInfoByIndex(int index, int &tex, Rect &rect);


private:
	std::string m_name;
	bool m_initialized;
	int m_frameNum;
	int m_numTextures;
	Texture *m_textures[MAX_TEXTURES];
	int m_textureSize;
	int m_chunkWidth, m_chunkHeight;
	TexFormat m_texFormat;
	int m_rowsPerTexture, m_colsPerTexture, m_atlasPerTexture;
	int m_totalAtlas;
	int *m_atlasId;		// an array stores each chunk's have been stored int
	int *m_atlasFrame;	
	Dict<int, int>	m_atlasHash;	// int -> indexes map
};

AX_END_NAMESPACE

#endif
