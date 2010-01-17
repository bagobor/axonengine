#ifndef AX_VARIANT_H
#define AX_VARIANT_H

AX_BEGIN_NAMESPACE

class Object;
class Variant;

//--------------------------------------------------------------------------
// class LuaTable
//--------------------------------------------------------------------------

struct AX_API LuaTable
{
public:
	LuaTable(int index);

	void beginRead() const;
	Variant get(const String &n) const;
	void set(const String &n, const Variant &v);
	int getLength() const;
	// n is start from 0
	Variant get(int n) const;
	void endRead() const;

	void beginIterator() const;
	bool nextIterator(Variant &k, Variant &v) const;
	void endIterator() const;

	Vector3 toVector3() const;
	Color3 toColor() const;
	Point toPoint() const;
	Rect toRect() const;
	Object *toObject() const;

public:
	int m_index;
	mutable bool m_isReading;
	mutable bool m_isIteratoring;
	mutable int m_stackTop;
};

//--------------------------------------------------------------------------
// class Variant
//--------------------------------------------------------------------------

class AX_API Variant
{
public:
	enum Type {
		kEmpty, kBool, kInt, kFloat, kString, kObject, kTable, kVector3, kColor3, kPoint, kRect, kMatrix3x4, kMaxType
	};

	enum {
		MINIBUF_SIZE = 4 * sizeof(float)
	};

	Type getType() const { return type; }

	// constructor
	Variant();
	Variant(bool v);
	Variant(int v);
	Variant(double v);
	Variant(const String &v);
	Variant(const char *v);
	Variant(Object *v);
	Variant(const Vector3 &v);
	Variant(const Point &v);
	Variant(const Rect &v);
	Variant(const Color3 &v);
	Variant(const Variant &v);
	Variant(const LuaTable &table);
	Variant(const Matrix3x4 &matrix);
	~Variant();

	void clear();
	operator bool() const;
	operator int() const;
	operator float() const;
	operator double() const;
	operator String() const;
	operator Object*() const;
	operator Vector3() const;
	operator Point() const;
	operator Rect() const;
	operator Color3() const;
	operator LuaTable() const;
	Variant &operator=(const Variant &v);
	operator Matrix3x4() const;

	void set(int v);
	void set(float v);
	void set(double v);
	void set(const char *v);
	void set(const String &v);
	void set(const Variant &v);
	void set(Object *v);

	bool toBool() const { return operator bool(); }
	int toInt() const { return operator int(); }
	float toFloat() const { return operator float(); }

	String toString() const;
	void fromString(Type t, const char *str);

	template<class Q>
	Q cast() {
		return variant_cast<Q>(*this);
	}

	static int getTypeSize(Type t);
	static bool canCast(Type fromType, Type toType);
	static bool rawCast(Type fromType, const void *fromData, Type toType, void *toData);

	// member variable
	Type type;
	union {
		bool boolval;
		int intval;
		double realval;
		Object *obj;
		String *str;
		Matrix3x4 *mtr;
		byte_t minibuf[MINIBUF_SIZE];
	};
};


typedef Sequence<Variant> VariantSeq;

template< typename T >
inline Variant::Type GetVariantType_() {
	// must be specialized, or raise a static error
	AX_STATIC_ASSERT(0);
}

template<>
inline Variant::Type GetVariantType_<void>() {
	return Variant::kEmpty;
}

template<>
inline Variant::Type GetVariantType_<int>() {
	return Variant::kInt;
}

template<>
inline Variant::Type GetVariantType_<bool>() {
	return Variant::kBool;
}

template<>
inline Variant::Type GetVariantType_<float>() {
	return Variant::kFloat;
}

template<>
inline Variant::Type GetVariantType_<String>() {
	return Variant::kString;
}

template<>
inline Variant::Type GetVariantType_<Vector3>() {
	return Variant::kVector3;
}

template<>
inline Variant::Type GetVariantType_<Color3>() {
	return Variant::kColor3;
}

template<>
inline Variant::Type GetVariantType_<Point>() {
	return Variant::kPoint;
}

template<>
inline Variant::Type GetVariantType_<Rect>() {
	return Variant::kRect;
}

template<>
inline Variant::Type GetVariantType_<Object*>() {
	return Variant::kObject;
}

template<>
inline Variant::Type GetVariantType_<Matrix3x4>() {
	return Variant::kMatrix3x4;
}

template<>
inline Variant::Type GetVariantType_<LuaTable>() {
	return Variant::kTable;
}

// variant cast
template< class T >
struct variant_cast_helper {
	T doCast(const Variant &v) {
		return v;
	}
};

template<>
struct variant_cast_helper<Object*> {
	Object *doCast(const Variant &v) {
		return v.operator Object*();
	}
};

template<class T>
struct variant_cast_helper<T*> {
	T *doCast(const Variant &v) {
		Object *obj = variant_cast_helper<Object*>().doCast(v);
		return object_cast<T*>(obj);
	}
};

template<class T>
T variant_cast(const Variant &v) {
	return variant_cast_helper<T>().doCast(v);
}
#if 0
template<>
Object *variant_cast<Object*>(const Variant &v) {
	return v.operator Object*();
}

template<class T>
T *variant_cast<T*>(const Variant &v) {
	Object *obj = variant_cast<Object*>(v);
	return 0;
}
#endif

AX_END_NAMESPACE

#include "variant.inl"

#endif // AX_VARIANT_H
