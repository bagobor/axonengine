#ifndef AX_SQUIRRELOBJECT_H
#define AX_SQUIRRELOBJECT_H

AX_BEGIN_NAMESPACE

class sqObject;
class sqVM;

class SquirrelError
{
public:
	SquirrelError(sqVM *vm);
	SquirrelError(const SQChar* s):desc(s){}
	const SQChar *desc;
};


class sqObject
{
	friend class sqVM;

public:
	sqObject();
	~sqObject();
	sqObject(const sqObject &o);
	sqObject(HSQOBJECT o);

	sqObject& operator=(const sqObject &o);
	void reset();

	SQObjectType getType() const;
	bool isClosure() const { return sq_isclosure(m_obj); }
	bool isNativeClosure() const { return sq_isnativeclosure(m_obj); }
	bool isCallable() const { return isClosure() || isNativeClosure(); }
	bool isClass() const { return sq_isclass(m_obj); }
	bool isNull() const { return sq_isnull(m_obj); }
	bool isNumeric() const { return sq_isnumeric(m_obj); }
	bool isInstance() const { return sq_isinstance(m_obj); }
	bool isArray() const { return sq_isarray(m_obj); }
	bool isTable() const { return sq_istable(m_obj); }

	operator HSQOBJECT&() const { return m_obj; } 
	bool operator==(const sqObject& o);
	bool compareUserPointer(const sqObject& o);

	void attachToStackObject(HSQUIRRELVM vm, int idx);
	sqObject clone();
	bool setValue(const sqObject &key, const sqObject &val);

	bool setValue(SQInteger key, const sqObject &val);
	bool setValue(int key, bool b); // Compiler treats SQBool as int.
	bool setValue(int key, int n);
	bool setValue(int key, float f);
	bool setValue(int key, const SQChar *s);

	bool setValue(const std::string &key, const sqObject &val);
	bool setValue(const std::string &key, bool b);
	bool setValue(const std::string &key, int n);
	bool setValue(const std::string &key, float f);
	bool setValue(const std::string &key, const SQChar *s);
	bool setValue(const std::string &key, const ConstRef &ref);

	bool setUserPointer(const std::string &key, SQUserPointer up);
	SQUserPointer getUserPointer(const std::string &key);
	bool setUserPointer(int key, SQUserPointer up);
	SQUserPointer getUserPointer(int key);

	bool newUserData(const std::string &key, int size, SQUserPointer * typetag=0);
	bool getUserData(const std::string &key, SQUserPointer * data, SQUserPointer * typetag=0);
	bool rawGetUserData(const std::string &key, SQUserPointer * data, SQUserPointer * typetag=0);

	// === BEGIN Arrays ===
	bool arrayResize(int newSize);
	bool arrayExtend(int amount);
	bool arrayReverse();
	sqObject arrayPop(SQBool returnPoppedVal=SQTrue);
	void arrayAppend(const sqObject &o);
	// === END Arrays ===

	bool setInstanceUP(SQUserPointer up);
	SQUserPointer getInstanceUP(SQUserPointer tag) const;
	int len() const;
	bool setDelegate(sqObject &obj);
	sqObject getDelegate();

	const SQChar* toString() const;
	bool toBool() const;
	SQInteger toInteger() const;
	SQFloat toFloat() const;
	sqObject getValue(const std::string &key) const;
	bool exists(const std::string &key) const;
	float getFloat(const std::string &key) const;
	int getInt(const std::string &key) const;
	const SQChar *getString(const std::string &key) const;
	bool getBool(const std::string &key) const;
	sqObject getValue(int key) const;
	float getFloat(int key) const;
	int getInt(int key) const;
	const SQChar *getString(int key) const;
	bool getBool(int key) const;
	sqObject getAttributes(const std::string &key = std::string());
	HSQOBJECT & getObjectHandle() const {return *(HSQOBJECT*)&m_obj;}
	bool beginIteration();
	bool next(sqObject &key, sqObject &value);
	void endIteration();

	bool getTypeTag(SQUserPointer * typeTag);
	bool setTypeTag(SQUserPointer typeTag);

	// === Return base class of object using sq_getbase() === 
	sqObject getBase();

	void toVariant(Variant &val) const;

protected:
	bool getSlot(const std::string &key) const { return getSlot(key.c_str(), key.size()); }
	bool getSlot(const SQChar *name, int size = -1) const;
	bool rawGetSlot(const std::string &key) const { return rawGetSlot(key.c_str(), key.size()); }
	bool rawGetSlot(const SQChar *name, int size = -1) const;
	bool getSlot(int key) const;

private:
	mutable HSQOBJECT m_obj;
};

struct ScopedStack
{
	ScopedStack(HSQUIRRELVM v)
	{
		m_top = sq_gettop(v);
		m_vm = v;
	}

	~ScopedStack()
	{
		sq_settop(m_vm, m_top);
	}

	HSQUIRRELVM m_vm;
	int m_top;
};

struct StackHandler
{
	StackHandler(HSQUIRRELVM v)
	{
		_top = sq_gettop(v);
		this->v = v;
	}

	SQFloat getFloat(int idx)
	{
		SQFloat x = 0.0f;
		if (idx > 0 && idx <= _top) {
			sq_getfloat(v, idx, &x);
		}
		return x;
	}

	SQInteger getInt(int idx)
	{
		SQInteger x = 0;
		if (idx > 0 && idx <= _top) {
			sq_getinteger(v, idx, &x);
		}
		return x;
	}

	HSQOBJECT getObjectHandle(int idx)
	{
		HSQOBJECT x;
		if (idx > 0 && idx <= _top) {
			sq_resetobject(&x);
			sq_getstackobj(v, idx, &x);
		}
		return x;
	}

	const SQChar *getString(int idx)
	{
		const SQChar *x = NULL;
		if (idx > 0 && idx <= _top) {
			sq_getstring(v, idx, &x);
		}
		return x;
	}

	SQUserPointer getUserPointer(int idx)
	{
		SQUserPointer x = 0;
		if (idx > 0 && idx <= _top) {
			sq_getuserpointer(v, idx, &x);
		}
		return x;
	}

	SQUserPointer getInstanceUp(int idx, SQUserPointer tag)
	{
		SQUserPointer self;
		if (SQ_FAILED(sq_getinstanceup(v, idx,(SQUserPointer*)&self, tag)))
			return NULL;
		return self;
	}

	SQUserPointer getUserData(int idx, SQUserPointer tag=0)
	{
		SQUserPointer otag;
		SQUserPointer up;
		if (idx > 0 && idx <= _top) {
			if(SQ_SUCCEEDED(sq_getuserdata(v, idx, &up,&otag))) {
				if(tag == otag)
					return up;
			}
		}
		return NULL;
	}

	bool getBool(int idx)
	{
		SQBool ret;
		if(idx > 0 && idx <= _top) {
			if(SQ_SUCCEEDED(sq_getbool(v, idx,&ret)))
				return ret;
		}
		return FALSE;
	}

	int getType(int idx)
	{
		if(idx > 0 && idx <= _top) {
			return sq_gettype(v, idx);
		}
		return -1;
	}

	int getParamCount() {
		return _top;
	}

	void getVariant(int idx, Variant &result);

	int Return(const ConstRef &arg);

	int Return(const SQChar *s)
	{
		sq_pushstring(v, s,-1);
		return 1;
	}

	int Return(float f)
	{
		sq_pushfloat(v, f);
		return 1;
	}

	int Return(int i)
	{
		sq_pushinteger(v, i);
		return 1;
	}

	int Return(bool b)
	{
		sq_pushbool(v, b);
		return 1;
	}

	int Return(SQUserPointer p) {
		sq_pushuserpointer(v, p);
		return 1;
	}

	int Return(sqObject &o)
	{
		sq_pushobject(v, o.getObjectHandle());
		return 1;
	}

	int Return() { return 0; }

	int throwError(const SQChar *error) {
		return sq_throwerror(v, error);
	}

	HSQUIRRELVM getVM() { return v; }

private:
	int _top;
	HSQUIRRELVM v;
};

AX_END_NAMESPACE

#endif // AX_SQUIRRELOBJECT_H
