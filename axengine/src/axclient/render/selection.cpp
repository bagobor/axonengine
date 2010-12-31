/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/
#include "../private.h"

#define DEFAULT_ID -111111111
#define DEFAULT_NUM_VERTEX (129 * 129)
#define INVALID_POS -1

// clip plane flags: �ü���λ��
#define LEFT_CLIP 0x0001
#define RIGHT_CLIP 0x0002
#define BOTTOM_CLIP 0x0004
#define TOP_CLIP 0x0008
#define NEAR_CLIP 0x0010
#define FAR_CLIP 0x0020

#ifdef _DEBUG
	#define CheckNumError(num)\
		if ((num) != (num))\
		{\
			Errorf(#num" is invalid!");\
		}	
#else
	#define CheckNumError(num)
#endif

AX_BEGIN_NAMESPACE

void aliasClipLeft (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);
void aliasClipRight (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);

void aliasClipTop (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);
void aliasClipBottom (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);

void aliasClipNear (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);
void aliasClipFar (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out);

void resizeSelectionVertexBuffer(SelectionVertex** buffer, int num);

struct SelectionVertex 
{
	float x;		// �ü��ռ�x����(-1.0f ~ 1.0f)
	float y;		// �ü��ռ�y����(-1.0f ~ 1.0f)
	float z;		// �ü��ռ�z����(-1.0f ~ 1.0f)

	Vector3 eyeCoor;// ������ϵ�������

	int flags;	// ��������Ϣ
};

static SelectionVertex *gSelectionVertexs = new SelectionVertex[DEFAULT_NUM_VERTEX];
static int gNumVertex = DEFAULT_NUM_VERTEX;
static SelectionVertex gAliasVertex[2][8];

static float zNear, zFar;
static float epision = 0.000001f;
static int numCalled = 0;

RenderCamera Selection::m_selectionCamera;

Selection::Selection(void)
	: m_isSelectMode(false)
	, m_idPos(INVALID_POS)
	, m_currentTestId(DEFAULT_ID)
	, m_isActor(false)
{
	
}

Selection::~Selection(void)
{
	SafeDeleteArray(gSelectionVertexs);
}

void Selection::beginSelect(const RenderCamera &view)
{
	m_selectTime = OsUtil::seconds();

	m_selectionCamera = view;

	zFar = view.getZfar();
	zNear = view.getZnear();

	m_isSelectMode = true;

	m_selectRecSeq.clear();
}

void Selection::loadSelectId(int id)
{
	m_currentTestId = id;
	m_idPos = INVALID_POS;
	m_isActor = false;
}

void Selection::testEntity(RenderEntity *re)
{
	// ������Ըþ����,����������ϵ�������.
	m_selectModelViewMatrix = m_selectionCamera.getViewMatrix() * re->getModelMatrix();
	m_isActor = true;

	Primitives prims = re->getHitTestPrims();
	Primitives::iterator it;

	for (it = prims.begin(); it != prims.end(); ++it)
	{
		Primitive *prim = *it;

		testPrimitive(prim);
	}
}

void Selection::testPrimitive(Primitive *prim)
{
	if (prim == NULL)
	{
		return ;
	}
	
	// �������Actor,���Ӿ����ó���ģ�;���
	if (m_isActor == false)
	{
		m_selectModelViewMatrix = m_selectionCamera.getViewMatrix();
	}

	// �ж�ͼԪ���Ͳ�����
	if (prim->getType() == Primitive::LineType)
	{
		LinePrim *line = static_cast<LinePrim*> (prim);

		testLine(line);
	}
	else if (prim->getType() == Primitive::MeshType)
	{
		MeshPrim *mesh = static_cast<MeshPrim*> (prim);

		testMesh(mesh);
	}
	else if (prim->getType() == Primitive::ChunkType)
	{
		ChunkPrim *chunk = static_cast<ChunkPrim*> (prim);

		testChunk(chunk);
	}
}

void Selection::testPrimitive(Primitive *prim, const Matrix &matrix)
{
	// �������Actor,���Ӿ����ó���ģ�;���
	m_selectModelViewMatrix = m_selectionCamera.getViewMatrix() * matrix.toMatrix4();

	// �ж�ͼԪ���Ͳ�����
	if (prim->getType() == Primitive::LineType)
	{
		LinePrim *line = static_cast<LinePrim*> (prim);

		testLine(line);
	}
	else if (prim->getType() == Primitive::MeshType)
	{
		MeshPrim *mesh = static_cast<MeshPrim*> (prim);

		testMesh(mesh);
	}
	else if (prim->getType() == Primitive::ChunkType)
	{
		ChunkPrim *chunk = static_cast<ChunkPrim*> (prim);

		testChunk(chunk);
	}
}

HitRecords Selection::endSelect()
{
	m_isSelectMode = false;
	m_currentTestId = DEFAULT_ID;
	m_idPos = INVALID_POS;
	m_isActor = false;

	// ��Ⲣ�����ڴ��С 
	++numCalled;

	// ÿ����100��,���һ���ڴ�
	if (numCalled >= 100)
	{
		numCalled = 0;

		// ����ڴ����Ĭ��, �����·���
		if (gNumVertex > DEFAULT_NUM_VERTEX)
		{
			resizeSelectionVertexBuffer(&gSelectionVertexs, DEFAULT_NUM_VERTEX);
		}
	}
	
	// ��ʾʰȡ���ѵ�ʱ��
	m_selectTime = OsUtil::seconds() - m_selectTime;

	Printf("select time: %f\n", m_selectTime);

	return m_selectRecSeq;
}

void Selection::testLine(const LinePrim *line)
{
	// ת�����������Ϊ��������һ������
	const DebugVertex *vertexs = line->getVertexesPointer();

	int numVexTranslate = line->getNumVertexes();

	if (numVexTranslate > gNumVertex)
	{
		resizeSelectionVertexBuffer(&gSelectionVertexs, numVexTranslate);
	}

	Vector3 v;
	for (int i=0; i<numVexTranslate; ++i)
	{
		translateToEyeCoor(vertexs[i].position, gSelectionVertexs[i].eyeCoor);

		CheckNumError(gSelectionVertexs[i].eyeCoor.x);
		CheckNumError(gSelectionVertexs[i].eyeCoor.y);
		CheckNumError(gSelectionVertexs[i].eyeCoor.z);

		gSelectionVertexs[i].flags = 0;

		// ���ж��Ƿ�Ҫ��Z���вü�
		if (gSelectionVertexs[i].eyeCoor.z > -zNear)
		{
			gSelectionVertexs[i].flags |= NEAR_CLIP;
		}
		else if (gSelectionVertexs[i].eyeCoor.z < -zFar)
		{
			gSelectionVertexs[i].flags |= FAR_CLIP;
		}
		else // ����Z�ü��Ķ���ֱ��ת����ͶӰ��һ������
		{
			translateToProCoor(gSelectionVertexs[i]);
			initVertexFlags(gSelectionVertexs[i]);

			CheckNumError(gSelectionVertexs[i].x);
			CheckNumError(gSelectionVertexs[i].y);
			CheckNumError(gSelectionVertexs[i].z);
		}
	}

	// �����������ݲ��ü�ͼԪ
	int numIndex = line->getActivedIndexes();

	if (numIndex == 0)
	{
		numIndex = line->getNumIndexes();
	}

	const ushort_t *indexes = line->getIndexPointer();

	for (int i=0; i<numIndex-1; i+=2)
	{
		if (indexes[i] < numVexTranslate && indexes[i+1] < numVexTranslate)
		{
			aliasClipLine(gSelectionVertexs[indexes[i]], gSelectionVertexs[indexes[i+1]]);
		}
	}
}

void Selection::testMesh(const MeshPrim *mesh)
{
	// ת�����������Ϊ��һ������
	const MeshVertex *vertexs = mesh->getVertexesPointer();

	int numVexTranslate = mesh->getNumVertexes();

	if (numVexTranslate > gNumVertex)
	{
		resizeSelectionVertexBuffer(&gSelectionVertexs, numVexTranslate);
	}

	Vector3 v;
	for (int i=0; i<numVexTranslate; ++i)
	{
		translateToEyeCoor(vertexs[i].position, gSelectionVertexs[i].eyeCoor);

		CheckNumError(gSelectionVertexs[i].eyeCoor.x);
		CheckNumError(gSelectionVertexs[i].eyeCoor.y);
		CheckNumError(gSelectionVertexs[i].eyeCoor.z);

		gSelectionVertexs[i].flags = 0;

		// ���ж��Ƿ�Ҫ��Z���вü�
		if (gSelectionVertexs[i].eyeCoor.z > -zNear)
		{
			gSelectionVertexs[i].flags |= NEAR_CLIP;
		}
		else if (gSelectionVertexs[i].eyeCoor.z < -zFar)
		{
			gSelectionVertexs[i].flags |= FAR_CLIP;
		}
		else // ����Z�ü��Ķ���ֱ��ת����ͶӰ��һ������
		{
			translateToProCoor(gSelectionVertexs[i]);

			CheckNumError(gSelectionVertexs[i].x);
			CheckNumError(gSelectionVertexs[i].y);
			CheckNumError(gSelectionVertexs[i].z);

			initVertexFlags(gSelectionVertexs[i]);
		}
	}

	// �����������ݲ��ü�ͼԪ
	int numIndex = mesh->getActivedIndexes();

	if (numIndex == 0)
	{
		numIndex = mesh->getNumIndexes();
	}

	const ushort_t *indexes = mesh->getIndexPointer();

	// ����Ǵ�״����
	if (mesh->isStriped())
	{
		for (int i=0; i<numIndex-2; ++i)
		{
			if (indexes[i] < numVexTranslate && indexes[i+1] < numVexTranslate && indexes[i+2] < numVexTranslate)
			{
				if (i % 2 == 0)
				{
					aliasClipTriangle(gSelectionVertexs[indexes[i]], gSelectionVertexs[indexes[i+1]], gSelectionVertexs[indexes[i+2]]);
				}
				else
				{
					aliasClipTriangle(gSelectionVertexs[indexes[i+1]], gSelectionVertexs[indexes[i]], gSelectionVertexs[indexes[i+2]]);
				}
			}
		}
	}
	else
	{
		for (int i=0; i+2<numIndex; i+=3)
		{
			if (indexes[i] < numVexTranslate && indexes[i+1] < numVexTranslate && indexes[i+2] < numVexTranslate)
			{
				aliasClipTriangle(gSelectionVertexs[indexes[i]], gSelectionVertexs[indexes[i+1]], gSelectionVertexs[indexes[i+2]]);
			}
		}
	}
}

void Selection::testChunk(const ChunkPrim *chunk)
{
	// ת�����������Ϊ��������һ������
	const ChunkVertex *vertexs = chunk->getVertexesPointer();
	int numVexTranslate = chunk->getNumVertexes();

	if (numVexTranslate > gNumVertex)
	{
		resizeSelectionVertexBuffer(&gSelectionVertexs, numVexTranslate);
	}
	
	Vector3 v;
	for (int i=0; i<numVexTranslate; ++i)
	{
		translateToEyeCoor(vertexs[i].position, gSelectionVertexs[i].eyeCoor);

		CheckNumError(gSelectionVertexs[i].eyeCoor.x);
		CheckNumError(gSelectionVertexs[i].eyeCoor.y);
		CheckNumError(gSelectionVertexs[i].eyeCoor.z);

		gSelectionVertexs[i].flags = 0;

		// ���ж��Ƿ�Ҫ��Z���вü�
		if (gSelectionVertexs[i].eyeCoor.z > -zNear)
		{
			gSelectionVertexs[i].flags |= NEAR_CLIP;
		}
		else if (gSelectionVertexs[i].eyeCoor.z < -zFar)
		{
			gSelectionVertexs[i].flags |= FAR_CLIP;
		}
		else // ����Z�ü��Ķ���ֱ��ת����ͶӰ��һ������
		{
			translateToProCoor(gSelectionVertexs[i]);

			CheckNumError(gSelectionVertexs[i].x);
			CheckNumError(gSelectionVertexs[i].y);
			CheckNumError(gSelectionVertexs[i].z);
			
			initVertexFlags(gSelectionVertexs[i]);
		}
	}

	// �����������ݲ��ü�ͼԪ
	int numIndex = chunk->getActivedIndexes();

	if (numIndex == 0)
	{
		numIndex = chunk->getNumIndexes();
	}

	const ushort_t *indexes = chunk->getIndexesPointer();

	for (int i=0; i<numIndex-2; i+=3)
	{
		if (indexes[i] < numVexTranslate && indexes[i+1] < numVexTranslate && indexes[i+2] < numVexTranslate)
		{
			aliasClipTriangle(gSelectionVertexs[indexes[i]], gSelectionVertexs[indexes[i+1]], gSelectionVertexs[indexes[i+2]]);
		}
	}
}

// ��ָ���������ν��вü�
void Selection::aliasClipTriangle(const SelectionVertex &vertex0, 
	const SelectionVertex &vertex1, const SelectionVertex &vertex2)
{
	// check side first, because most material is one-side
	Vector3 v0(vertex0.x, vertex0.y, vertex0.z);
	Vector3 v1(vertex1.x, vertex1.y, vertex1.z);
	Vector3 v2(vertex2.x, vertex2.y, vertex2.z);

	Vector3 t = v1 - v0;
	Vector3 b = v2 - v1;
	Vector3 n = t ^ b;

	if (n.z > 0)
		return;

	// ���3������ı�־λ�߼���Ϊ��, ���ͼԪ��ȫ�ڲü���Χ����
	if (vertex0.flags & vertex1.flags & vertex2.flags)
	{
		return ;
	}
	else if (!(vertex0.flags | vertex1.flags | vertex2.flags))	// ���3������ı�־λ�߼���Ϊ��, ���ͼԪ��ȫ�ڲü���Χ����
	{
		HitRecord record;

		record.name = m_currentTestId;

		record.minz = std::min(std::min(vertex0.z, vertex1.z), vertex2.z);
		record.maxz = std::max(std::max(vertex0.z, vertex1.z), vertex2.z);

		addSelectionRecord(record);

		return ;
	}
	else // ���ݶ���Ŀռ���λ�����ü�
	{
		int i, k, pingpong = 0;
		unsigned clipflags;

		gAliasVertex[pingpong][0] = vertex0;
		gAliasVertex[pingpong][1] = vertex1;
		gAliasVertex[pingpong][2] = vertex2;

		// ����߶ε�һ��������λ���ĳλΪ1,����һ���������ͬλΪ0,����߶δ�����Ӧ�Ĳü��߽�.
		clipflags = gAliasVertex[pingpong][0].flags | gAliasVertex[pingpong][1].flags | gAliasVertex[pingpong][2].flags;

		// ����Ҫ�ж��Ƿ���Z�ü�,�еĻ������������ﴦ��,Ȼ����ת����ͶӰ��һ������
		if ((clipflags & NEAR_CLIP) || (clipflags & FAR_CLIP))
		{
			if (clipflags & NEAR_CLIP)
			{
				k = aliasClip(gAliasVertex[0], gAliasVertex[1], NEAR_CLIP, 3, aliasClipNear);

				if (k == 0)
				{
					return;
				}

				pingpong ^= 1;
			}

			//clipflags = gAliasVertex[1][0].flags | gAliasVertex[1][1].flags | gAliasVertex[1][2].flags;

			if (clipflags & FAR_CLIP)
			{
				k = aliasClip(gAliasVertex[0], gAliasVertex[1], FAR_CLIP, 3, aliasClipFar);

				if (k == 0)
				{
					return;
				}

				pingpong ^= 1;
			}

			clipflags = gAliasVertex[pingpong][0].flags | gAliasVertex[pingpong][1].flags | gAliasVertex[pingpong][2].flags;
		}
		else
		{
			pingpong = 0;
			k = 3;		// ��ʼ������
		}

		if (clipflags & LEFT_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				LEFT_CLIP, k, aliasClipLeft);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & RIGHT_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				RIGHT_CLIP, k, aliasClipRight);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & BOTTOM_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				BOTTOM_CLIP, k, aliasClipBottom);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & TOP_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				TOP_CLIP, k, aliasClipTop);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		// �洢�ü����
		HitRecord record;

		record.maxz = -1.0f;
		record.minz = 1.0f;
		record.name = m_currentTestId;

		for (i=0; i<k; i++)
		{
			record.minz = std::min(gAliasVertex[pingpong][i].z, record.minz);
			record.maxz = std::max(gAliasVertex[pingpong][i].z, record.maxz);
		}

		addSelectionRecord(record);
	}
}

// �߶βü�
void Selection::aliasClipLine(const SelectionVertex &vertex0, const SelectionVertex &vertex1)
{
	// ���2������ı�־λ�߼���Ϊ��, ���ͼԪ��ȫ�ڲü���Χ����
	if (vertex0.flags & vertex1.flags)
	{
		return ;
	}
	else if (!(vertex0.flags | vertex1.flags))	// ���2������ı�־λ�߼���Ϊ��, ���ͼԪ��ȫ�ڲü���Χ����
	{
		HitRecord record;

		record.name = m_currentTestId;

		record.minz = std::min(vertex0.z, vertex1.z);
		record.maxz = std::max(vertex0.z, vertex1.z);

		addSelectionRecord(record);

		return ;
	}
	else // ���ݶ���Ŀռ���λ�����ü�
	{
		int i, k, pingpong = 0;
		unsigned clipflags;

		gAliasVertex[pingpong][0] = vertex0;
		gAliasVertex[pingpong][1] = vertex1;

		// ����߶ε�һ��������λ���ĳλΪ1,����һ���������ͬλΪ0,����߶δ�����Ӧ�Ĳü��߽�.
		clipflags = gAliasVertex[pingpong][0].flags | gAliasVertex[pingpong][1].flags;

		// ����Ҫ�ж��Ƿ���Z�ü�,�еĻ������������ﴦ��,Ȼ����ת����ͶӰ��һ������
		if ((clipflags & NEAR_CLIP) || (clipflags & FAR_CLIP))
		{
			if (clipflags & NEAR_CLIP)
			{
				k = aliasClip(gAliasVertex[0], gAliasVertex[1], NEAR_CLIP, 2, aliasClipNear);

				if (k == 0)
				{
					return;
				}

				pingpong ^= 1;
			}

			if (clipflags & FAR_CLIP)
			{
				k = aliasClip(gAliasVertex[0], gAliasVertex[1], FAR_CLIP, 2, aliasClipFar);

				if (k == 0)
				{
					return;
				}

				pingpong ^= 1;
			}

			clipflags = gAliasVertex[pingpong][0].flags | gAliasVertex[pingpong][1].flags;
		}
		else
		{
			pingpong = 0;
			k = 2;		// ��ʼ������
		}

		if (clipflags & LEFT_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				LEFT_CLIP, k, aliasClipLeft);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & RIGHT_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				RIGHT_CLIP, k, aliasClipRight);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & BOTTOM_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				BOTTOM_CLIP, k, aliasClipBottom);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		if (clipflags & TOP_CLIP)
		{
			k = aliasClip (gAliasVertex[pingpong], gAliasVertex[pingpong ^ 1],
				TOP_CLIP, k, aliasClipTop);

			if (k == 0)
			{
				return;
			}

			pingpong ^= 1;
		}

		// �洢�ü����
		HitRecord record;

		record.maxz = -1.0f;
		record.minz = 1.0f;
		record.name = m_currentTestId;

		for (i=0; i<k; i++)
		{
			record.minz = std::min(gAliasVertex[pingpong][i].z, record.minz);
			record.maxz = std::max(gAliasVertex[pingpong][i].z, record.maxz);
		}

		addSelectionRecord(record);
	}
}

// ��������ת���ɹ�һ��ͶӰ����
void Selection::translateToProCoor(SelectionVertex &vexter)
{
	Vector3 v;

	v = m_selectionCamera.getProjMatrix() * vexter.eyeCoor;

	vexter.x = v.x;
	vexter.y = v.y;
	vexter.z = v.z;
}

// ����������ת����������
void Selection::translateToEyeCoor(const Vector3 &inVertex, Vector3 &outVertex)
{
	outVertex = m_selectModelViewMatrix * inVertex;	// ��ģ������ת����������
}

// ����һ����¼,����һ��������һ����¼,Ҳ�������滻ԭ���ļ�¼.
void Selection::addSelectionRecord(const HitRecord &record)
{
	// ����ǵ�һ��, ������һ����¼
	if (m_idPos == INVALID_POS)
	{
		m_idPos = (int) m_selectRecSeq.size();
		m_selectRecSeq.push_back(record);
	}
	else // ����Ƚϲ��滻ԭ�еļ�¼
	{
		if (record.minz < m_selectRecSeq[m_idPos].minz)
		{
			m_selectRecSeq[m_idPos].minz = record.minz;
		}

		if (record.maxz > m_selectRecSeq[m_idPos].maxz)
		{
			m_selectRecSeq[m_idPos].maxz = record.maxz;
		}
	}
}

// ��ʼ��������λ���־.����Z�ü������������Ѿ��ж�,�ʲ��ڴ˴��ж�
void Selection::initVertexFlags(SelectionVertex &vertex)
{
	vertex.flags = 0;

	if (vertex.x < -1.0f)
	{
		vertex.flags |= LEFT_CLIP;
	}
	else if (vertex.x > 1.0f)
	{
		vertex.flags |= RIGHT_CLIP;
	}

	if (vertex.y < -1.0f)
	{
		vertex.flags |= BOTTOM_CLIP;
	}
	else if (vertex.y > 1.0f)
	{
		vertex.flags |= TOP_CLIP;
	}
}

// �Զ���ν��вü�, count>=2������ȷ�ü�.���زü���Ķ�����.
int Selection::aliasClip(SelectionVertex *in, SelectionVertex *out, int flag, int count,
		void (*clip) (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out))
{
	int i, j, k;
	int flags, oldflags;

	j = count-1;
	k = 0;

	for (i=0 ; i<count ; j = i, i++)
	{
		oldflags = in[j].flags & flag;
		flags = in[i].flags & flag;

		if (flags && oldflags)
		{
			continue;
		}

		if (oldflags ^ flags)
		{
			// ���òü�
			clip(&in[j], &in[i], &out[k]);

			// ��ʼ��־λ
			initVertexFlags(out[k]);

			k++;
		}

		if (!flags)
		{
			out[k] = in[i];
			k++;
		}
	}

	return k;	// ���زü���Ķ������
}

void aliasClipLeft (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->x - pfv1->x) < epision)
	{
		out->x = -1.0f;
		out->y = pfv0->y;
		out->z = pfv0->z;
		
		return ;
	}

	float scale;

	if (pfv0->x >= pfv1->x)
	{
		scale =(-1.0f - pfv0->x) / (pfv1->x - pfv0->x);

		out->x = -1.0f;
		out->y = pfv0->y + (pfv1->y - pfv0->y) * scale;
		out->z = pfv0->z + (pfv1->z - pfv0->z) * scale;
	}
	else
	{
		scale =(-1.0f - pfv1->x) / (pfv0->x - pfv1->x);

		out->x = -1.0f;
		out->y = pfv1->y + (pfv0->y - pfv1->y) * scale;
		out->z = pfv1->z + (pfv0->z - pfv1->z) * scale;
	}

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

void aliasClipRight (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->x - pfv1->x) < epision)
	{
		out->x = 1.0f;
		out->y = pfv0->y;
		out->z = pfv0->z;

		return ;
	}

	float scale;

	if (pfv0->x >= pfv1->x)
	{
		scale =(1.0f - pfv0->x) / (pfv1->x - pfv0->x);

		out->x = 1.0f;
		out->y = pfv0->y + (pfv1->y - pfv0->y) * scale;
		out->z = pfv0->z + (pfv1->z - pfv0->z) * scale;
	}
	else
	{
		scale =(1.0f - pfv1->x) / (pfv0->x - pfv1->x);

		out->x = 1.0f;
		out->y = pfv1->y + (pfv0->y - pfv1->y) * scale;
		out->z = pfv1->z + (pfv0->z - pfv1->z) * scale;
	}

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

void aliasClipTop (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->y - pfv1->y) < epision)
	{
		out->x = pfv0->x;
		out->y = 1.0f;
		out->z = pfv0->z;

		return ;
	}

	float scale;

	if (pfv0->y >= pfv1->y)
	{
		scale =(1.0f - pfv0->y) / (pfv1->y - pfv0->y);

		out->x = pfv0->x + (pfv1->x - pfv0->x) * scale;
		out->y = 1.0f;
		out->z = pfv0->z + (pfv1->z - pfv0->z) * scale;
	}
	else
	{
		scale =(1.0f - pfv1->y) / (pfv0->y - pfv1->y);

		out->x = pfv1->x + (pfv0->x - pfv1->x) * scale;
		out->y = 1.0f;
		out->z = pfv1->z + (pfv0->z - pfv1->z) * scale;
	}

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

void aliasClipBottom (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->y - pfv1->y) < epision)
	{
		out->x = pfv0->x;
		out->y = -1.0f;
		out->z = pfv0->z;

		return ;
	}

	float scale;

	if (pfv0->y >= pfv1->y)
	{
		scale =(-1.0f - pfv0->y) / (pfv1->y - pfv0->y);

		out->x = pfv0->x + (pfv1->x - pfv0->x) * scale;
		out->y = -1.0f;
		out->z = pfv0->z + (pfv1->z - pfv0->z) * scale;
	}
	else
	{
		scale =(-1.0f - pfv1->y) / (pfv0->y - pfv1->y);

		out->x = pfv1->x + (pfv0->x - pfv1->x) * scale;
		out->y = -1.0f;
		out->z = pfv1->z + (pfv0->z - pfv1->z) * scale;
	}

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

void aliasClipNear (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->eyeCoor.z - pfv1->eyeCoor.z) < epision)
	{
		out->eyeCoor.x = pfv0->eyeCoor.x;
		out->eyeCoor.y = pfv0->eyeCoor.y;
		out->eyeCoor.z = -zNear;

		Selection::translateToProCoor(*out);
		Selection::initVertexFlags(*out);

		return ;
	}

	float scale;

	if (pfv0->eyeCoor.z >= pfv1->eyeCoor.z)
	{
		scale =(-zNear - pfv0->eyeCoor.z) / (pfv1->eyeCoor.z - pfv0->eyeCoor.z);

		out->eyeCoor.x = pfv0->eyeCoor.x + (pfv1->eyeCoor.x - pfv0->eyeCoor.x) * scale;
		out->eyeCoor.y = pfv0->eyeCoor.y + (pfv1->eyeCoor.y - pfv0->eyeCoor.y) * scale;
		out->eyeCoor.z = -zNear;
	}
	else
	{
		scale =(-zNear - pfv1->eyeCoor.z) / (pfv0->eyeCoor.z - pfv1->eyeCoor.z);

		out->eyeCoor.x = pfv1->eyeCoor.x + (pfv0->eyeCoor.x - pfv1->eyeCoor.x) * scale;
		out->eyeCoor.y = pfv1->eyeCoor.y + (pfv0->eyeCoor.y - pfv1->eyeCoor.y) * scale;
		out->eyeCoor.z = -zNear;
	}

	Selection::translateToProCoor(*out);

	Selection::initVertexFlags(*out);

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

void aliasClipFar (SelectionVertex *pfv0, SelectionVertex *pfv1, SelectionVertex *out)
{
	if (fabs(pfv0->eyeCoor.z - pfv1->eyeCoor.z) < epision)
	{
		out->eyeCoor.x = pfv0->eyeCoor.x;
		out->eyeCoor.y = pfv0->eyeCoor.y;
		out->eyeCoor.z = -zFar;

		Selection::translateToProCoor(*out);
		//Selection::initVertexFlags(*out);

		return ;
	}

	float scale;

	if (pfv0->eyeCoor.z >= pfv1->eyeCoor.z)
	{
		scale =(-zFar - pfv0->eyeCoor.z) / (pfv1->eyeCoor.z - pfv0->eyeCoor.z);

		out->eyeCoor.x = pfv0->eyeCoor.x + (pfv1->eyeCoor.x - pfv0->eyeCoor.x) * scale;
		out->eyeCoor.y = pfv0->eyeCoor.y + (pfv1->eyeCoor.y - pfv0->eyeCoor.y) * scale;
		out->eyeCoor.z = -zFar;
	}
	else
	{
		scale =(-zFar - pfv1->eyeCoor.z) / (pfv0->eyeCoor.z - pfv1->eyeCoor.z);

		out->eyeCoor.x = pfv1->eyeCoor.x + (pfv0->eyeCoor.x - pfv1->eyeCoor.x) * scale;
		out->eyeCoor.y = pfv1->eyeCoor.y + (pfv0->eyeCoor.y - pfv1->eyeCoor.y) * scale;
		out->eyeCoor.z = -zFar;
	}

	Selection::translateToProCoor(*out);

	//Selection::initVertexFlags(*out);

	CheckNumError(out->x);
	CheckNumError(out->y);
	CheckNumError(out->z);
}

// ���·��䶥��buffer�Ĵ�С
void resizeSelectionVertexBuffer(SelectionVertex** buffer, int num)
{
	AX_ASSERT(num > 0);

	// �ǵ�Ҫ���ͷ�
	if (*buffer != NULL)
	{
		//memcpy();
		delete []*buffer;
	}

	*buffer = new SelectionVertex[num];

	gNumVertex = num;
}

AX_END_NAMESPACE