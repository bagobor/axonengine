#ifndef AX_OBJECT_H
#define AX_OBJECT_H

AX_BEGIN_NAMESPACE

//--------------------------------------------------------------------------
// class Object
//--------------------------------------------------------------------------

class File;
class Object;
typedef Object *ObjectStar;

class AX_API Object
{
	friend class ScriptSystem;
	friend class SqProperty;

public:

	Object();
	virtual ~Object();

	virtual CppClass *getCppClass() const;
	virtual std::string getNamespace() const { return std::string(); }
	static CppClass *registerCppClass();
	const ScriptClass *getScriptClass() const { return m_scriptClass; }

	// if have scriptclass, return script class name; otherwise rethrn cppclass name
	const FixedString & getClassName() const
	{
		if (m_scriptClass) return m_scriptClass->getName();
		return getCppClass()->getName();
	}

	Member *findMember(const FixedString &name) const;
	bool getProperty(const FixedString &name, Variant &ret) const;
	bool getProperty(const FixedString &name, const Ref &ret) const;
	bool setProperty(const FixedString &name, const ConstRef &value);
	bool setProperty(const FixedString &name, const char *value);

	// properties
	void set_objectName(const std::string &name);
	std::string get_objectName() const;

	bool inherits(const FixedString &cls) const;

	// read and write properties
	void writeProperties(File *f, int indent=0) const;
	void readProperties(const TiXmlElement *node);
	void copyPropertiesFrom(const Object *rhs);

	// runtime
	void setRuntime(const FixedString &name, const Variant &val);
	Variant getRuntime(const FixedString &name);

	void doPropertyChanged();

	bool invokeMethodRt(const FixedString &methodName, const Ref &ret=Ref(), const ConstRef &arg0=ConstRef(), const ConstRef &arg1=ConstRef(), const ConstRef &arg2=ConstRef(), const ConstRef &arg3=ConstRef(), const ConstRef &arg4=ConstRef());
	bool invokeMethod(const FixedString &methodName, const ConstRef &arg0=ConstRef(), const ConstRef &arg1=ConstRef(), const ConstRef &arg2=ConstRef(), const ConstRef &arg3=ConstRef(), const ConstRef &arg4=ConstRef());
	bool invokeScriptRt(const FixedString &methodName, const Ref &ret=Ref(), const ConstRef &arg0=ConstRef(), const ConstRef &arg1=ConstRef(), const ConstRef &arg2=ConstRef(), const ConstRef &arg3=ConstRef(), const ConstRef &arg4=ConstRef());
	bool invokeScript(const FixedString &methodName, const ConstRef &arg0=ConstRef(), const ConstRef &arg1=ConstRef(), const ConstRef &arg2=ConstRef(), const ConstRef &arg3=ConstRef(), const ConstRef &arg4=ConstRef());

	bool isScriptInstanced() const { return !m_scriptInstance.isNull(); }
	ScriptValue& getScriptInstance() { return m_scriptInstance; }

protected:
	virtual void onPropertyChanged();

	void setObjectName(const std::string &name);
	void resetObjectName();

	void raiseSignal(const std::string &signal, const Variant &param);
	void initScriptClass(const ScriptClass *sqclass);

protected:
	const ScriptClass *m_scriptClass;

	std::string m_objectNamespace;
	std::string m_objectName;

	ScriptValue m_scriptInstance;

	ScriptValue m_currentState;
};

AX_END_NAMESPACE

#endif // AX_OBJECT_H