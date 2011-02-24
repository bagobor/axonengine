/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#ifndef AX_RENDER_ACTOR_H
#define AX_RENDER_ACTOR_H

AX_BEGIN_NAMESPACE

// forward declaration
struct RenderScene;

class QuadNode;

class AX_API RenderEntity
{
	friend class RenderWorld;
	friend class QuadNode;
	friend class BvhNode;

public:
	enum Flag {
		DepthHack = 1, OutsideOnly = 2, Static = 4
	};

	enum Kind {
		kNone, kModel, kSpeedTree, kAnchor, kEffect, kUserInterface,
		kLight, kFog, kVisArea, kPortal, kOccluder, kTerrain, kOutdoorEnv
	};

	RenderEntity(Kind type);
	virtual ~RenderEntity();

	Kind getKind() const { return m_kind; }

	// read and writable
	const Vector3 &getOrigin() const;
	void setOrigin(const Vector3 &origin);
	const Matrix3 &getAxis() const;
	void setAxis(const Angles &angles);
	void setAxis(const Angles &angles, float scale);
	const Matrix &getMatrix() const;
	void setMatrix(const Matrix &mat);
	const Vector4 &getInstanceParam() const { return m_instanceParam; }

	void setInstanceColor(const Vector3 &color);
	Vector3 getInstanceColor() const;

	bool isInWorld() const { return m_world != 0; }
	void refresh();

	int getFlags() const;
	void setFlags(int flags);
	void addFlags(int flags);
	bool isFlagSet(Flag flag) const;
	void setStatic(bool value) { addFlags(Static); }
	bool isStatic() const { return isFlagSet(Static); }

	bool isVisible() const;
	bool isLight() const { return m_kind == kLight; }
	float getVisSize() const { return m_visSize; }
	float getDistance() const { return m_distance; }

	// read only
	Matrix4 getModelMatrix() const;

	RenderWorld *getWorld() const { return m_world; }
	void setWorld(RenderWorld *world) { m_world = world; }

	// helper prims
	void clearHelperPrims();
	void addHelperPrim(Primitive *prim);
	const Primitives &getHelperPrims() const;

	void interactionChain(Interaction *last, int chainId);

	// virtual interface
	virtual BoundingBox getLocalBoundingBox() = 0;
	virtual BoundingBox getBoundingBox() = 0;
	virtual Primitives getHitTestPrims() { return Primitives(); }
	virtual void frameUpdate(RenderScene *scene);
	virtual void issueToScene(RenderScene *scene) {}

protected:
	// only called by RenderWorld
	void update(RenderScene *qscene, bool allInFrustum);
	void updateCsm(RenderScene *qscene, bool allInFrustum);
	bool isCsmCulled() const;
	void calculateLod(RenderScene *qscene, bool allInFrustum);

protected:
	const Kind m_kind;
	Matrix m_affineMat;
	int m_flags;
	Vector4 m_instanceParam;

	int m_visFrameId;
	BoundingBox m_linkedBbox;
	float m_linkedExtends;
	float m_distance;
	float m_lodRatio;
	float m_lod;
	float m_visSize;
	bool m_viewDistCulled;
	bool m_queryCulled;

	// linked info
	IntrusiveLink<RenderEntity> m_nodeLink;
	IntrusiveLink<RenderEntity> m_worldLink;
	QuadNode *m_linkedNode;
	int m_linkedFrame;

	// helper primitives
	Primitives m_helperPrims;

	// used by world
	RenderWorld *m_world;
	Query *m_visQuery;
	Query *m_shadowQuery;

	// used by interaction
	int m_chainId;
	Interaction *m_headInteraction;
};

inline void RenderEntity::clearHelperPrims()
{
	m_helperPrims.clear();
}

inline void RenderEntity::addHelperPrim(Primitive *prim)
{
	m_helperPrims.push_back(prim);
}

inline const Primitives &RenderEntity::getHelperPrims() const
{
	return m_helperPrims;
}

inline void RenderEntity::interactionChain(Interaction *last, int chainId)
{
	if (m_chainId != chainId) {
		m_headInteraction = last;
		m_chainId = chainId;
		return;
	}

	last->actorNext = m_headInteraction;
	m_headInteraction = last;
}


//--------------------------------------------------------------------------
// class IEntityManager
//--------------------------------------------------------------------------

class AX_API IEntityManager {
public:
	virtual bool isSupportExt(const std::string &ext) const = 0;
	virtual RenderEntity *create(const std::string &name, intptr_t arg = 0) = 0;
	virtual void updateForFrame(RenderScene *qscene ) {}
	virtual void issueToScene(RenderScene *qscene) {}
};

AX_END_NAMESPACE

#endif // AX_RENDER_ACTOR_H
