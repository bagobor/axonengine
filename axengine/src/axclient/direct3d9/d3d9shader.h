/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_D3D9SHADER_H
#define AX_D3D9SHADER_H

#define AX_SU(x, y) D3D9uniform::setUniform(Uniforms::x,y);

AX_BEGIN_NAMESPACE

class D3D9Shader;

//--------------------------------------------------------------------------
// class UniformCache
//--------------------------------------------------------------------------

class D3D9uniform : public UniformItem
{
public:
	D3D9uniform(UniformItem &item, D3DXHANDLE param);
	virtual ~D3D9uniform();

	bool isCached() const;
	void cache();

	template< class Q >
	static void setUniform(Uniforms::ItemName itemname, const Q &q) {
		UniformItem &item = g_uniforms.getItem(itemname);

		if (item.m_valueType == UniformItem::vt_Texture) {
			item.set(q);
			return;
		}

		if (memcmp(&q, item.m_datap, item.m_dataSize) == 0)
			return;

		item.set(q);

		setUniform(item, &q);
	}

	static void setUniform(UniformItem &item, const void *q);

public:
	UniformItem *m_src;
	D3DXHANDLE m_param;
};

//--------------------------------------------------------------------------
// class CGsamplerann
//--------------------------------------------------------------------------

class D3D9SamplerAnn : public SamplerAnno
{
public:
	FixedString m_paramName;
	D3DXHANDLE m_param;
	int m_register;
};
typedef Sequence<D3D9SamplerAnn*>	D3D9SamplerAnns;

//--------------------------------------------------------------------------
// class D3D9pixeltotexel
//--------------------------------------------------------------------------

class D3D9Pixel2Texel
{
public:
	String m_name;
	D3DXHANDLE m_param;
	FloatSeq m_pixelValue;
	FloatSeq m_scaledValue;
};
typedef Sequence<D3D9Pixel2Texel>	D3D9Pixel2Texels;

class D3D9Pass
{
public:
	friend class D3D9Shader;
	
	struct ParamDesc {
		D3DXCONSTANT_DESC d3dDesc;
		const D3D9Pixel2Texel *p2t;
	};

	D3D9Pass(D3D9Shader *shader, D3DXHANDLE d3dxhandle);
	~D3D9Pass();

	void begin();

protected:
	void initVs();
	void initPs();
	void initState();
	void initSampler(const D3DXCONSTANT_DESC &desc);

	const D3D9Pixel2Texel *findPixel2Texel(const String &name);
	void setParameters();
	void setParameter(const ParamDesc &param, const float *value, bool isPixelShader);

private:
	D3D9Shader *m_shader;
	D3DXHANDLE m_d3dxhandle;

	// shader
	IDirect3DVertexShader9 *m_vs;
	IDirect3DPixelShader9 *m_ps;

	// render state
	DWORD m_depthTest;
	DWORD m_depthWrite;
	DWORD m_cullMode;
	DWORD m_blendEnable;
	DWORD m_blendSrc;
	DWORD m_blendDst;

	// material sampler
	int m_matSamplers[SamplerType::NUMBER_ALL];

	// sys sampler
	Dict<int,int> m_sysSamplers;

	// batch sampler
	D3D9SamplerAnns m_batchSamplers;

	// local parameter
	Dict<String,ParamDesc> m_vsParameters;
	Dict<String,ParamDesc> m_psParameters;
};

class D3D9Technique
{
public:
	friend class D3D9Shader;

	D3D9Technique(D3D9Shader *shader, D3DXHANDLE d3dxhandle);
	~D3D9Technique();

private:
	enum {MAX_PASSES = 8};
	D3D9Shader *m_shader;
	D3DXHANDLE m_d3dxhandle;

	int m_numPasses;
	D3D9Pass *m_passes[MAX_PASSES];
};

//--------------------------------------------------------------------------
// class D3D9Shader
//--------------------------------------------------------------------------

class D3D9Shader : public Shader
{
public:
	friend class D3D9Pass;
	friend class D3D9Technique;

	D3D9Shader();
	virtual ~D3D9Shader();

	// implement Shader
	virtual bool doInit(const String &name, const ShaderMacro &macro = g_shaderMacro);
	virtual bool isDepthWrite() const;
	virtual bool haveTextureTarget() const;
	virtual int getNumSampler() const;
	virtual SamplerAnno *getSamplerAnno(int index) const;
	virtual int getNumTweakable() const;
	virtual ParameterAnno *getTweakableDef(int index);
	virtual SortHint getSortHint() const;
	virtual bool haveTechnique(Technique tech) const;
	virtual const ShaderInfo *getShaderInfo() const { return 0; }

	void setSystemMap(SamplerType maptype, D3D9Texture *tex);
	// set pixel to texel conversion paramter
	void setPixelToTexel(int width, int height);

	void setCoupled(Material *mtr);

	ID3DXEffect *getObject() const { return m_object; }

	UINT begin(Technique tech);
	void beginPass(UINT pass);
	void endPass();
	void end();

protected:
	void initTechniques();
	void initFeatures();
	void initSortHint();

	void initAnnotation();
	void initSamplerAnn(D3DXHANDLE param);
	void initParameterAnn(D3DXHANDLE param);
	void initPixelToTexel(D3DXHANDLE param);

	void initAxonObject();

	D3DXHANDLE findTechnique(Technique tech);
	D3DXHANDLE getUsedParameter(const char *name);
	bool isParameterUsed(D3DXHANDLE param);

private:
	LPD3DXEFFECT m_object;              // Effect object
	String m_keyString;
	SortHint m_sortHint;

	D3DXHANDLE m_d3dxTechniques[Technique::Number];
	D3D9Texture *m_samplerBound[SamplerType::NUMBER_ALL];
	D3DXHANDLE m_curTechnique;

	bool m_haveTextureTarget;

	D3D9SamplerAnns m_samplerannSeq;

	// pixel2texel
	D3D9Pixel2Texels pixel2Texels;
	int m_p2tWidth, m_p2tHeight;

	D3D9Technique *m_techniques[Technique::Number];
	D3D9Technique *m_curTech;
	Material *m_coupled;

	// shader info
	ShaderInfo *m_shaderInfo;
};

//--------------------------------------------------------------------------
// class D3D9shadermanager
//--------------------------------------------------------------------------

class D3D9ShaderManager : public ShaderManager
{
public:
	D3D9ShaderManager();
	virtual ~D3D9ShaderManager();

	virtual Shader *findShader(const String &name, const ShaderMacro &macro = g_shaderMacro);
	virtual Shader *findShader(const FixedString &nameId, const ShaderMacro &macro);
	virtual void saveShaderCache(const String &name);
	virtual void applyShaderCache(const String &name);

	D3D9Shader *findShaderDX(const String &name, const ShaderMacro &macro = g_shaderMacro);

protected:
	void _initialize();

private:
	typedef Dict<FixedString,Dict<ShaderMacro,D3D9Shader*> > ShaderPool;
	ShaderPool m_shaderPool;
	D3D9Shader *m_defaulted;
	ShaderInfoDict m_shaderInfoDict;
};


AX_END_NAMESPACE


#endif // end guardian

