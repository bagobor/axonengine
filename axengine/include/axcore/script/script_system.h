/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/




#ifndef AX_SCRIPTSYSTEM_H
#define AX_SCRIPTSYSTEM_H

#define AX_DECLARE_CLASS(classname, baseclass) public: \
	typedef classname ThisClass; \
	typedef baseclass BaseClass; \
	virtual CppClass *classname::getCppClass() const { \
		return classname::registerCppClass(); \
	} \
	static CppClass *classname::registerCppClass() { \
		static CppClass *typeinfo; \
		if (!typeinfo) { \
			typeinfo = new CppClass_<classname>(#classname, BaseClass::registerCppClass());

#define AX_CONSTPROP(name) typeinfo->addProperty(#name, &ThisClass::get_##name);
#define AX_PROP(name) typeinfo->addProperty(#name, &ThisClass::get_##name, &ThisClass::set_##name);
#define AX_SIMPLEPROP(name) typeinfo->addProperty(#name, &ThisClass::m_##name);

#define AX_METHOD(name) typeinfo->addMethod(#name, &ThisClass::name);

#define AX_END_CLASS() \
				g_scriptSystem->registerCppClass(typeinfo); \
			} \
		return typeinfo; \
	}


#define AX_REGISTER_CLASS(cppname) cppname::registerCppClass();


AX_BEGIN_NAMESPACE

//--------------------------------------------------------------------------
// class ScriptSystem
//--------------------------------------------------------------------------

class ScriptSystem;
extern AX_API ScriptSystem *g_scriptSystem;

class AX_API ScriptSystem
{
public:
	friend class Object;

	ScriptSystem();
	~ScriptSystem();

	void initialize();
	void finalize();

	void executeLine(const char *text);

	Object *createObject(const FixedString &classname);
	Object *cloneObject(const Object *obj);

	// for automatic name gen
	int getNameIndex(const std::string &str) const;
	void updateNameIndex(const std::string &str);
	int nextNameIndex(const std::string &str);
	std::string generateObjectName(const std::string &str);

	void registerCppClass(CppClass *metainfo);
	void registerScriptClass(const std::string &name);

	void getClassList(const char *prefix, bool sort, StringSeq &result) const;

	// connect signal and slot
	bool connect(Object *sender, const std::string &sig, Object *recevier, const std::string &slot);
	bool disconnect(Object *sender, const std::string &sig, Object *recevier, const std::string &slot);

	CppClass *findCppClass(const char *name) const;
	ScriptClass *findScriptClass(const char *name) const;

	void allocThread(ScriptThread &thread);

	static ScriptValue createMetaClosure(Member *method);

protected:
	void linkCppToScript(CppClass *ti);

private:
	typedef Dict<FixedString, ScriptClass*> ScriptClassDict;
	ScriptClassDict m_scriptClassReg;

	typedef Dict<FixedString, CppClass*> CppClassDict;
	CppClassDict m_cppClassReg;

	typedef Dict<std::string,int> StringIntDict;
	StringIntDict m_objectNameGen;

	bool m_isReading;
	int m_readTop;
};


template< class T >
T object_cast(Object *obj) {
	if (obj->inherits(T(0)->registerCppClass()->getName())) {
		return (T)obj;
	}
	return nullptr;
}

AX_END_NAMESPACE

#endif // AX_SCRIPTSYSTEM_H

