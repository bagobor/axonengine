#include "script_p.h"

AX_BEGIN_NAMESPACE

namespace {

	class Handler : public Variant::TypeHandler
	{
	public:
		Handler()
		{
			m_dataSize = sizeof(ScriptValue);
		}

		virtual bool canCast(Variant::TypeId toType)
		{
			return true;
		}

		virtual bool rawCast(const void *fromData, Variant::TypeId toType, void *toData)
		{
			const ScriptValue &from = *reinterpret_cast<const ScriptValue*>(fromData);
			return from.rawCast(toType, toData);
		}

		virtual void construct(void *ptr, const void *copyfrom)
		{
			new (ptr) ScriptValue(*reinterpret_cast<const ScriptValue*>(copyfrom));
		}
		virtual void construct(void *ptr)
		{
			new (ptr) ScriptValue();
		}

		virtual void destruct(void *ptr)
		{
			reinterpret_cast<ScriptValue*>(ptr)->~ScriptValue();
		}
	};

	Handler s_scriptValueHandler;
}

ScriptValue::ScriptValue() : m_d(new sqObject())
{}

ScriptValue::ScriptValue(const sqObject &sobj) : m_d(new sqObject(sobj))
{}

ScriptValue::ScriptValue(const ScriptValue &rhs) : m_d(new sqObject(*rhs.m_d))
{}

ScriptValue::~ScriptValue()
{
	SafeDelete(m_d);
}

bool ScriptValue::rawCast(Variant::TypeId toType, void *toData) const
{
	Variant val;
	m_d->toVariant(val);

	return val.castTo(toType, toData);
}

Variant::TypeHandler * ScriptValue::getTypeHandler()
{
	return &s_scriptValueHandler;
}

ScriptValue& ScriptValue::operator=(const ScriptValue &rhs)
{
	*m_d = *rhs.m_d;
	return *this;
}

bool ScriptValue::isNull() const
{
	return m_d->isNull();
}

bool ScriptValue::isInstance() const
{
	return m_d->isInstance();
}

AX_END_NAMESPACE
