/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "../private.h"

AX_BEGIN_NAMESPACE

static const float SKYBOX_SIZE = 4096;
static const float SKYBOX_TEXOFFSET = 1.0f / 512.0f;
static const float SKYDOME_RADIUS = 4096;
static const int SKYDOME_TESS = 24;
static const float ZER = SKYBOX_TEXOFFSET;
static const float ONE = 1.0f - ZER;
static const float OCEAN_RADIUS = 4096;
static const int OCEAN_SUBDIVIDE = 32 * 4;
static const float OCEAN_MULTIPLY = 1.3f;

OutdoorEnv::OutdoorEnv(RenderWorld *world) : RenderEntity(kOutdoorEnv) {
	m_world = world;

	m_haveSky = false;
	m_nishitaSky  = false;
	m_haveOcean = false;
	m_haveGlobalLight = true;

	m_dateTimeInited = false;

	createSkyBox();
	createSkyDome();
	setSkyBoxTexture("textures/skys/day/box");

	createOceanMesh();

	m_globalLight = new RenderLight(RenderLight::kGlobal, Vector3(1,-1,1), Rgb::White);
	m_globalLight->setWorld(m_world);

	// global fog
	m_globalFog = new RenderFog();
	m_globalFog->setFogDensity(0.0004f);
	m_globalFog->setFogColor(Rgba(119,171,201).toVector4());
	m_globalFog->setWorld(m_world);

	// ocean fog
	m_oceanFog = new RenderFog();
	m_oceanFog->setFogDensity(0.04f);
	m_oceanFog->setFogColor(Rgba(119,201,171).toVector4() * 0.25f);
	m_oceanFog->setWorld(m_world);

	TypeZero(&m_lastNishitaParams);
}

OutdoorEnv::~OutdoorEnv()
{}

void OutdoorEnv::createSkyBox() {
	// create box face 12
	static float l_verts12[20][5] = {
		// box12
		SKYBOX_SIZE, -SKYBOX_SIZE, 0,			ZER, 0.5,
		SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE, ZER, ZER,
		SKYBOX_SIZE, SKYBOX_SIZE, 0,			ONE, 0.5,
		SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,	ONE, ZER,
		SKYBOX_SIZE, SKYBOX_SIZE, 0,			ZER, 0.5,
		SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,	ZER, ONE,
		-SKYBOX_SIZE, SKYBOX_SIZE, 0,			ONE, 0.5,
		-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE, ONE, ONE,

		// box34
		-SKYBOX_SIZE, SKYBOX_SIZE, 0,			ZER, 0.5,
		-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE, ZER, ZER,
		-SKYBOX_SIZE, -SKYBOX_SIZE, 0,			ONE, 0.5,
		-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,ONE, ZER,
		-SKYBOX_SIZE, -SKYBOX_SIZE, 0,			ZER, 0.5,
		-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,ZER, ONE,
		SKYBOX_SIZE, -SKYBOX_SIZE, 0,			ONE, 0.5,
		SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE, ONE, ONE,

		// box5
		SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE, ZER, ONE,
		-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,ZER, ZER,
		SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,	ONE, ONE,
		-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE, ONE, ZER,
	};

	static ushort_t l_idxes[] = {
		0, 2, 1, 1, 2, 3,
		4, 6, 5, 5, 6, 7
	};

	// box12
	m_skybox12 = new MeshPrim(MeshPrim::HintStatic);
	m_skybox12->init(8, 12);
	MeshVertex *verts = m_skybox12->lockVertexes();
	for (int i = 0; i < 8; i++) {
		verts[i].xyz.set(l_verts12[i]);
		verts[i].st.set(l_verts12[i][3], l_verts12[i][4]);
		verts[i].rgba.set(255, 255, 255, 255);
	}
	m_skybox12->unlockVertexes();

	ushort_t *idxes = m_skybox12->lockIndexes();
	memcpy(idxes, l_idxes, 12 * sizeof(ushort_t));
	m_skybox12->unlockIndexes();

	// box34
	m_skybox34 = new MeshPrim(MeshPrim::HintStatic);
	m_skybox34->init(8, 12);
	verts = m_skybox34->lockVertexes();
	for (int i = 0; i < 8; i++) {
		verts[i].xyz.set(l_verts12[i+8]);
		verts[i].st.set(l_verts12[i+8][3], l_verts12[i+8][4]);
		verts[i].rgba.set(255, 255, 255, 255);
	}
	m_skybox34->unlockVertexes();

	idxes = m_skybox34->lockIndexes();
	memcpy(idxes, l_idxes, 12 * sizeof(ushort_t));
	m_skybox34->unlockIndexes();

	// box side 5
	m_skybox5 = new MeshPrim(MeshPrim::HintStatic);
	m_skybox5->init(4, 6);

	verts = m_skybox5->lockVertexes();
	for (int i = 0; i < 4; i++) {
		verts[i].xyz.set(l_verts12[i+16]);
		verts[i].st.set(l_verts12[i+16][3], l_verts12[i+16][4]);
		verts[i].rgba.set(255, 255, 255, 255);
	}
	m_skybox5->unlockVertexes();

	idxes = m_skybox5->lockIndexes();
	memcpy(idxes, l_idxes, 6 * sizeof(ushort_t));
	m_skybox5->unlockIndexes();
}

void OutdoorEnv::setSkyBoxTexture(const std::string &matname)
{
	if (m_skyBoxMatName == matname)
		return;

	m_skyBoxMatName = matname;
	{
		Material *skymat = m_skybox12->getMaterial();
		if (!skymat) {
			skymat = new Material("_skybox");
		}
		Texture *tex = new Texture(matname + "_12");
		AX_ASSERT(tex);
		skymat->setTexture(MaterialTextureId::Diffuse, tex);

		m_skybox12->setMaterial(skymat);
	}
	{
		Material *skymat = m_skybox34->getMaterial();
		if (!skymat) {
			skymat = new Material("_skybox");
		}
		Texture *tex = new Texture(matname + "_34");
		AX_ASSERT(tex);
		skymat->setTexture(MaterialTextureId::Diffuse, tex);

		m_skybox34->setMaterial(skymat);
	}
	{
		Material *skymat = m_skybox5->getMaterial();
		if (!skymat) {
			skymat = new Material("_skybox");
		}
		Texture *tex = new Texture(matname + "_5");
		AX_ASSERT(tex);
		skymat->setTexture(MaterialTextureId::Diffuse, tex);

		m_skybox5->setMaterial(skymat);
	}
}

void OutdoorEnv::createSkyDome()
{
	int tess = SKYDOME_TESS;
	int halftess = tess / 2;
	float radius = SKYDOME_RADIUS;

	int numverts = (tess + 1) * (halftess+1);
	int numidxes = tess * halftess * 2 * 3;

	m_skydome = new MeshPrim(MeshPrim::HintStatic);
	m_skydome->init(numverts, numidxes);

	// fill vertexes
	MeshVertex *verts = m_skydome->lockVertexes();
	MeshVertex *vertsStart = verts;
	memset(verts, 0, sizeof(MeshVertex) * numverts);

	for (int i = 0; i <= halftess; i++) {
		float y = (float)i / halftess;
		float beta = AX_PI_2 *(y * 1.1f);
		float bs, bc;
		float vertbeta = AX_PI_2 * i / halftess;
		float vbs, vbc;
		Math::sincos(beta, bs, bc);
		Math::sincos(vertbeta, vbs, vbc);

		for (int j = 0; j <= tess; j++) {
			float alpha = AX_PI * 2.0f / tess * j;
			float as, ac;
			Math::sincos(alpha, as, ac);

			verts->xyz.x = vbs * ac;
			verts->xyz.y = vbs * as;
			verts->xyz.z = vbc;
			verts->normal.x = bs * ac;
			verts->normal.y = bs * as;
			verts->normal.z = bc;
			verts->xyz *= radius;
			verts->st.set(j / (float)tess, i / (float)halftess);
			verts->rgba.set(255,255,255,255);
			verts++;
		}
	}

	AX_ASSERT(verts - vertsStart == numverts);
	m_skydome->unlockVertexes();

	// fill indexes
	ushort_t *idxes = m_skydome->lockIndexes();
	ushort_t *idxesStart = idxes;

	for (int i = 0; i < halftess; i++) {
		for (int j = 0; j < tess; j++) {
			int p0 = i *(tess + 1) + j;
			int p1 = p0 + 1;
			int p2 = (i+1) *(tess + 1) + j;
			int p3 = p2 + 1;

			idxes[0] = p0;
			idxes[1] = p2;
			idxes[2] = p1;
			idxes[3] = p1;
			idxes[4] = p2;
			idxes[5] = p3;
			idxes += 6;
		}
	}

	AX_ASSERT(idxes - idxesStart == numidxes);
	m_skydome->unlockIndexes();
#if 0
	Material *mat = Material::loadUnique("null");
	Texture *tex = new Texture("textures/testlalo");
	mat->setTexture(SamplerType::Diffuse, tex);
	m_skydome->setMaterial(mat);
#endif
	// create nishita render target
	m_skyNishitaRt = nullptr;

	if (!g_renderDriverInfo.textureFormatSupports[TexFormat::RGBA16F]) {
		return;
	}

	m_skyNishitaRt = new RenderTarget(128, 64, TexFormat::RGBA16F);
	RenderTarget *miert = new RenderTarget(128, 64, TexFormat::RGBA16F);

	SamplerDesc desc;
	desc.clampMode = SamplerDesc::ClampMode_Clamp;
	m_skyNishitaRt->getTexture()->setSamplerState(desc);
	miert->getTexture()->setSamplerState(desc);
	m_skyNishitaRt->attachColor(0, miert);

	m_skyNishitaMat = new Material("_skyNishita");
	m_skyNishitaMat->setTexture(MaterialTextureId::Diffuse, m_skyNishitaRt->getTexture());
	m_skyNishitaMat->setTexture(MaterialTextureId::Specular, miert->getTexture());
	m_skyNishitaGenMat = 0; //Material::loadUnique("_skyNishitaGen");

	m_skydome->setMaterial(m_skyNishitaMat);
}

void OutdoorEnv::createOceanMesh()
{
	m_oceanMesh = nullptr;

	// calculate vertexbuffer and indexbuffer size
	float r = 1.0f;
	float num = 1;
	while (r < OCEAN_RADIUS) {
		num++;
		r *= OCEAN_MULTIPLY;
	}

	int numverts = num * OCEAN_SUBDIVIDE + 1; // + 1 for center
	int numidxes =(num - 1) * OCEAN_SUBDIVIDE * 2 * 3 + OCEAN_SUBDIVIDE * 3;

	// create render mesh
	m_oceanMesh = new MeshPrim(MeshPrim::HintStatic);
	m_oceanMesh->init(numverts, numidxes);

	// init render mesh's vertexbuffer and indexbuffer
	MeshVertex *verts = m_oceanMesh->lockVertexes();
	MeshVertex *oldverts = verts;

	// clear to zero
	memset(verts, 0, sizeof(MeshVertex) * numverts);

	// first vertex is zero center
	verts->xyz.set(0, 0, 0);
	verts->normal.set(0, 0, 1);
	verts->tangent.set(1, 0, 0);
	verts->binormal.set(0, 1, 0);
	verts->st.set(0, 0); //  verts->st = verts->xyz.xy() * 0.01f;
	verts->rgba.set(255, 255, 255, 255);
	verts++;

	// inner vertexes
	const float angelStep = AX_PI * 2.0f / OCEAN_SUBDIVIDE;
	r = 1.0f;
	for (int i = 0; i < num; i++, r *= OCEAN_MULTIPLY) {
		for (int j = 0; j < OCEAN_SUBDIVIDE; j++) {
			float angle = j * angelStep;
			float s, c;
			Math::sincos(angle, s, c);
			verts->xyz.set(r * c, r * s, 0);
			verts->normal.set(0, 0, 1);
			verts->tangent.set(1, 0, 0);
			verts->binormal.set(0, 1, 0);
			verts->st.set(0, 0); // verts->st = verts->xyz.xy() * 0.01f;
			verts->rgba.set(255, 255, 255, 255);
			if (i == num - 1) {
				verts->st.set(0, 1);
			}
			verts++;
		}
	}

	m_oceanMesh->unlockVertexes();
	AX_ASSERT(verts - oldverts == numverts);

	// fill index buffer
	ushort_t *idxes = m_oceanMesh->lockIndexes();
	ushort_t *oldidxes = idxes;

	// innermost triangles
	for (int i = 0; i < OCEAN_SUBDIVIDE; i++) {
		*idxes++ = 0;
		*idxes++ =((i + 1) % OCEAN_SUBDIVIDE) + 1;
		*idxes++ = i + 1;
	}

	for (int i = 0; i < num-1; i++) {
		int start = i * OCEAN_SUBDIVIDE + 1;
		for (int j = 0; j < OCEAN_SUBDIVIDE; j++) {
			int p0 = start + j;
			int p1 = start +((j + 1) % OCEAN_SUBDIVIDE);
			int p2 = p0 + OCEAN_SUBDIVIDE;
			int p3 = p1 + OCEAN_SUBDIVIDE;

			*idxes++ = p0;
			*idxes++ = p1;
			*idxes++ = p2;
			*idxes++ = p2;
			*idxes++ = p1;
			*idxes++ = p3;
		}
	}
	m_oceanMesh->unlockIndexes();

	AX_ASSERT(idxes - oldidxes == numidxes);

	Material *mat = new Material("ocean");
	m_oceanMesh->setMaterial(mat);
}

void OutdoorEnv::createOceanGrid()
{}

BoundingBox OutdoorEnv::getLocalBoundingBox()
{
	BoundingBox result;
	result.min.set(0,0,0);
	result.max.set(1,1,1);
	return result;
}

BoundingBox OutdoorEnv::getBoundingBox()
{
	return getLocalBoundingBox();
}

#if 0
Primitives OutdoorEnv::getAllPrimitives()
{
	Primitives result;
	if (r_sky->getBool()) {
#if 1
		result.push_back(m_skybox12);
		result.push_back(m_skybox34);
		result.push_back(m_skybox5);
#else
		result.push_back(m_skydome);
#endif
	}

	if (r_water->getBool()) {
		result.push_back(m_oceanMesh);
	}
	return result;
}
#endif

void OutdoorEnv::frameUpdate(RenderScene *qscene)
{
	if (!m_dateTimeInited) {
		m_dateTime.initSystemTime(qscene->camera.getTime() * 1000);
		m_dateTimeInited = true;
	} else {
		m_dateTime.update(qscene->camera.getTime() * 1000);
	}

	DateTime::Data data = m_dateTime.getData();

	Vector3 viewdir = qscene->camera.getViewAxis()[0];
	m_wind.advance(qscene->camera.getTime(), true, false, viewdir.x, viewdir.y, viewdir.z);

	m_wind.getWindMatrix(qscene->windMatrices);
	m_wind.getLeafAngles(qscene->leafAngles);
#if 0
	genNishitaUpdateScene(camera, qscenes);
#endif
}


void OutdoorEnv::issueToQueue(RenderScene *qscene)
{
	if (m_haveGlobalLight) {
		qscene->addEntity(this->getGlobalLight());
		getGlobalLight()->issueToQueue(qscene);
	}

	qscene->addEntity(m_globalFog);
	qscene->globalFog = m_globalFog;

	if (m_haveOcean) {
		qscene->addEntity(m_oceanFog);
		qscene->waterFog = m_oceanFog;
	}

	if (qscene->camera.getOrigin().z <= 0) {
		qscene->globalFog = qscene->waterFog;
		qscene->isEyeInWater = true;
	}

	float exposure = qscene->exposure;
	if (exposure == 0) {
		exposure = 1;
	}
	qscene->clearColor = m_oceanFog->getFogColor() / exposure;

	bool havesky = m_haveSky && r_sky.getBool();
	bool haveocean = m_haveOcean && r_water.getBool();
	bool nishitaSky = havesky && r_sky.getInteger() > 1;

	if (havesky) {
		if (0 && nishitaSky) {
			NishitaParams curparam;
			curparam.sunPos = m_globalLight->getOrigin().getNormalized();
			if (m_lastNishitaParams != curparam) {
				m_lastNishitaParams = curparam;
				genNishitaUpdateScene(qscene);
			}
			qscene->addInteraction(this, m_skydome);
		} else {
			qscene->addInteraction(this, m_skybox12);
			qscene->addInteraction(this, m_skybox34);
			qscene->addInteraction(this, m_skybox5);
		}
	}

	if (haveocean) {
		qscene->addInteraction(this, m_oceanMesh);
	}
#if 0
	PrimitiveSeq prims = this->getViewedPrimitives();

	for (size_t i = 0; i < prims.size(); i++) {
		qscene->addInteraction(this, prims[i]);
	}
#endif
}


void OutdoorEnv::genNishitaUpdateScene(RenderScene *qscene)
{
	RenderScene *scene = qscene->addSubScene();
	if (!scene)
		return;

	scene->sceneType = RenderScene::RenderToTexture;
	scene->camera = qscene->camera;

	scene->camera.setTarget(m_skyNishitaRt);
	scene->camera.setOverlay(0, 0, 128, 64);

	MeshPrim *quad = MeshPrim::createScreenQuad(Primitive::HintFrame, Rect(0,0,128,64), Rgba::White, m_skyNishitaGenMat);
	scene->addInteraction(nullptr, quad);
}

RenderLight *OutdoorEnv::getGlobalLight() const
{
	return m_globalLight;
}

void OutdoorEnv::setHaveOcean(bool have)
{
	m_haveOcean = have;
}

void OutdoorEnv::setHaveFarSky(bool have)
{
	m_haveSky = have;
}

void OutdoorEnv::setFog(const Vector3 &color, float density)
{
	m_globalFog->setFogColor(color);
	m_globalFog->setFogDensity(density);
}

void OutdoorEnv::setOceanFog(const Vector3 &color, float density)
{
	m_oceanFog->setFogColor(color);
	m_oceanFog->setFogDensity(density);
}

void OutdoorEnv::setOceanMaterial(const std::string &matname)
{}

void OutdoorEnv::setSunColor(const Color3 &color, float intensity, float specularX)
{
	m_globalLight->setLightColor(color, intensity, specularX);
}

void OutdoorEnv::setSunDir(const Vector3 &dir)
{
	m_globalLight->setOrigin(dir);
}

void OutdoorEnv::setSkyColor(const Color3 &color, float intensity)
{
	m_globalLight->setSkyColor(color, intensity);
}

void OutdoorEnv::setEnvColor(const Color3 &color, float intensity)
{
	m_globalLight->setEnvColor(color, intensity);
}

void OutdoorEnv::setHaveGlobalLight( bool have )
{
	m_haveGlobalLight = have;
}

void OutdoorEnv::setCastShadow( bool val )
{
	m_globalLight->setCastShadowMap(val);
}

AX_END_NAMESPACE

