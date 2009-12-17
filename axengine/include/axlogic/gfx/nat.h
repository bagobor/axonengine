/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#ifndef AX_GFX_NAT_H
#define AX_GFX_NAT_H

namespace Axon {

	// interpolation functions
	template<class T>
	inline T interpolate(const float r, const T &v1, const T &v2)
	{
		return static_cast<T>(v1*(1.0f - r) + v2*r);
	}

	template<class T>
	inline T interpolateHermite(const float r, const T &v1, const T &v2, const T &in, const T &out)
	{
		// basis functions
		float h1 = 2.0f*r*r*r - 3.0f*r*r + 1.0f;
		float h2 = -2.0f*r*r*r + 3.0f*r*r;
		float h3 = r*r*r - 2.0f*r*r + r;
		float h4 = r*r*r - r*r;

		// interpolation
		return static_cast<T>(v1*h1 + v2*h2 + in*h3 + out*h4);
	}

	// "linear" interpolation for quaternions should be slerp by default
	template<>
	inline Quaternion interpolate<Quaternion>(const float r, const Quaternion &v1, const Quaternion &v2)
	{
		return v1.slerp(v2, r);
	}

	template <class T>
	class Nat
	{
	public:
		enum InterpolateType
		{
			InterpolateType_None,
			InterpolateType_Linear,
			InterpolateType_Cubic,
			InterpolateType_Hermite,
		};

		typedef struct {
			int time;
			T val;
		} KeyValue;

		T getValue(int time);

		InterpolateType m_interpolateType;
		FixedString m_name;
		Sequence<KeyValue> m_keyValues;
		bool m_loop;
	};

	//--------------------------------------------------------------------------
	struct KeyBase
	{
		int ticks;
	};

	//--------------------------------------------------------------------------
	struct TcbKey : public KeyBase
	{
		float tension, continuity, bias, easeIn, easeOut;
	};

	//--------------------------------------------------------------------------
	struct FloatKey : public KeyBase
	{
		float val;
	};

	//--------------------------------------------------------------------------
	struct VectorKey : public KeyBase
	{
		Vector3 val;
	};

	//--------------------------------------------------------------------------
	struct ColorKey : public VectorKey
	{
		float alpha;
	};


	//--------------------------------------------------------------------------
	class IAnimatable
	{
	public:
		enum AnimType
		{
			kSimpleTrack,
			kTcbTrack,
			kBezierTrack,
			kLastTrack = kBezierTrack,
			kGroupAnim,
			kObjectAnim,
			kGameEntityAnim
		};

		// for simple track
		enum InterpolateType
		{
			InterpolateType_Invalid,
			InterpolateType_Step,
			InterpolateType_Linear,
			InterpolateType_CatmullRom,
		};

		virtual ~IAnimatable() {}

		virtual String getAnimName() = 0;
		virtual AnimType getAnimType() = 0;

		// for group, object, entity
		virtual int numSubAnims() { return 0; }
		virtual IAnimatable* getSubAnim(int index) { return 0; }

		// for track
		virtual int numKeys() { return 0; }
		virtual int getKeyTime(int index) { return 0; }
		virtual int getKeyIndex(int ms) { return 0; }
		virtual void getValue(void *value, int ticks) {}
		virtual void setValue(void *value, int ticks) {}

		// for simple track
		virtual InterpolateType getInterpolateType() { return InterpolateType_Invalid; }
		virtual void setInterpolateType(InterpolateType type) { /* do nothing */}
#if 0
		// for test
		virtual void beginUpdateAnim();
		virtual void endUpdateAnim();
#endif
	};


	//--------------------------------------------------------------------------
	class FloatTrack : public IAnimatable
	{
	public:
		FloatTrack(const String& name);
		virtual ~FloatTrack();

		// implement IAnimatable
		virtual String getAnimName() { return m_name; }
		virtual AnimType getAnimType() { return kSimpleTrack; }

		// implement track functions
		virtual int numKeys();
		virtual int getKeyTime(int index);
		virtual int getKeyIndex(int ms);
		virtual void getValue(void *value, int ticks);
		virtual void setValue(void *value, int ticks);

		// for simple track
		virtual InterpolateType getInterpolateType() { return m_interpolateType; }
		virtual void setInterpolateType(InterpolateType type) { m_interpolateType = type; }

	private:
		String m_name;
		Sequence<FloatKey> m_keyValues;
		InterpolateType m_interpolateType;
	};

	//--------------------------------------------------------------------------
	class VectorTrack : public IAnimatable
	{
	public:
		VectorTrack(const String& name);
		virtual ~VectorTrack();

		// implement IAnimatable
		virtual String getAnimName() { return m_name; }
		virtual AnimType getAnimType() { return kSimpleTrack; }

		// implement track functions
		virtual int numKeys();
		virtual int getKeyTime(int index);
		virtual int getKeyIndex(int ms);
		virtual void getValue(void *value, int ticks);
		virtual void setValue(void *value, int ticks);

		// for simple track
		virtual InterpolateType getInterpolateType() { return m_interpolateType; }
		virtual void setInterpolateType(InterpolateType type) { m_interpolateType = type; }

	private:
		String m_name;
		Sequence<VectorKey> m_keyValues;
		InterpolateType m_interpolateType;
	};

	//--------------------------------------------------------------------------
	class ColorTrack : public VectorTrack
	{
	public:
		ColorTrack(const String& name);
		virtual ~ColorTrack();

	private:
	};

	//--------------------------------------------------------------------------
	class ObjAnimatable : public IAnimatable
	{
	public:
		ObjAnimatable();
		virtual ~ObjAnimatable();

		virtual String getAnimName();
		virtual AnimType getAnimType() { return kObjectAnim; }

		// for group, object, entity
		virtual int numSubAnims() { return m_subAnims.size(); }
		virtual IAnimatable* getSubAnim(int index) { return m_subAnims[index]; }

	protected:
		Object* m_object;
		Sequence<IAnimatable*> m_subAnims;
	};


} // namespace Axon

#endif
