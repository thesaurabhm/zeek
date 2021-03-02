// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include "zeek/Desc.h"
#include "zeek/script_opt/CPPFunc.h"
#include "zeek/script_opt/ScriptOpt.h"


namespace zeek::detail {

// Helper class that tracks distinct instances of a given key.  T1 is the
// pointer version of the type and T2 the IntrusivePtr version.
template <class T1, class T2>
class CPPTracker {
public:
	CPPTracker(const char* _base_name)
	: base_name(_base_name)
		{
		}

	bool HasKey(T1 key) const	{ return map.count(key) > 0; }
	bool HasKey(T2 key) const	{ return HasKey(key.get()); }

	// Only adds the key if it's not already present.
	void AddKey(T2 key);

	std::string KeyName(T1 key);
	std::string KeyName(T2 key)	{ return KeyName(key.get()); }

	int KeyIndex(T1 key)	{ return map2[map[key]]; }
	int KeyIndex(T2 key)	{ return map2[map[key.get()]]; }

	const std::vector<T2>& Keys() const		{ return keys; }
	const std::vector<T2>& DistinctKeys() const	{ return keys2; }

	int Size() const		{ return keys.size(); }
	int DistinctSize() const	{ return keys2.size(); }

	const T1& GetRep(T2 key) 	{ return reps[map[key.get()]]; }

	hash_type Hash(T2 key) const;

private:
	// Maps keys to internal representations.
	std::unordered_map<T1, hash_type> map;

	// Maps internal representations to distinct values.
	std::unordered_map<hash_type, int> map2;

	// Tracks the set of keys, to facilitate iterating over them.
	// Parallel to "map".
	std::vector<T2> keys;

	// Similar, but only for "representative" keys, i.e., those
	// associated with distinct slots in map2.
	std::vector<T2> keys2;

	// Maps internal names to representatives.
	std::unordered_map<hash_type, T1> reps;

	// Used to construct key names.
	std::string base_name;
};

class CPPCompile {
public:
	CPPCompile(std::vector<FuncInfo>& _funcs) : funcs(_funcs) { }

	void CompileTo(FILE* f);
	void CompileTo(FILE* f, unsigned int addl_tag);

private:
	void GenProlog();
	void GenEpilog();

	bool IsCompilable(const FuncInfo& func);

	void DeclareGlobals(const FuncInfo& func);
	void AddBiF(const Func* b);
	void AddGlobal(const std::string& g, const char* suffix);
	void AddConstant(const ConstExpr* c);

	void DeclareFunc(const FuncInfo& func);
	void CompileFunc(const FuncInfo& func);

	void DeclareSubclass(const FuncInfo& func, const std::string& fname);
	void GenSubclassTypeAssignment(Func* f);
	void GenInvokeBody(const std::string& fname, const TypePtr& t,
				const std::string& args);

	void DefineBody(const FuncInfo& func, const std::string& fname);

	std::string BindArgs(const FuncTypePtr& ft);

	void DeclareLocals(const FuncInfo& func);

	std::string BodyName(const FuncInfo& func);

	void GenStmt(const StmtPtr& s)	{ GenStmt(s.get()); }
	void GenStmt(const Stmt* s);

	enum GenType {
		GEN_DONT_CARE,
		GEN_NATIVE,
		GEN_VAL_PTR,
	};

	std::string GenExpr(const ExprPtr& e, GenType gt, bool top_level = false)
		{ return GenExpr(e.get(), gt, top_level); }
	std::string GenExpr(const Expr* e, GenType gt, bool top_level = false);

	std::string GenArgs(const Expr* e);

	std::string GenUnary(const Expr* e, GenType gt, const char* op);
	std::string GenBinary(const Expr* e, GenType gt, const char* op);
	std::string GenBinarySet(const Expr* e, GenType gt, const char* op);
	std::string GenBinaryString(const Expr* e, GenType gt, const char* op);
	std::string GenBinaryPattern(const Expr* e, GenType gt, const char* op);
	std::string GenBinaryAddr(const Expr* e, GenType gt, const char* op);
	std::string GenBinarySubNet(const Expr* e, GenType gt, const char* op);
	std::string GenEQ(const Expr* e, GenType gt, const char* op);

	std::string GenAssign(const ExprPtr& lhs, const ExprPtr& rhs,
				const std::string& rhs_native,
				const std::string& rhs_val_ptr);

	std::string GenIntVector(const std::vector<int>& vec);

	std::string NativeToGT(const std::string& expr, const TypePtr& t,
				GenType gt);
	std::string GenericValPtrToGT(const std::string& expr, const TypePtr& t,
					GenType gt);

	void GenInitExpr(const ExprPtr& e);
	bool IsSimpleInitExpr(const ExprPtr& e) const;
	std::string InitExprName(const ExprPtr& e);

	void GenAttrs(const AttributesPtr& attrs);
	std::string AttrsName(const AttributesPtr& attrs);
	const char* AttrName(const AttrPtr& attr);

	void GenPreInit(const TypePtr& t);

	void ExpandTypeVar(const TypePtr& t);

	std::string GenTypeName(const TypePtr& t);

	const char* TypeTagName(TypeTag tag) const;

	const char* IDName(const ID& id)	{ return IDName(&id); }
	const char* IDName(const IDPtr& id)	{ return IDName(id.get()); }
	const char* IDName(const ID* id)	{ return IDNameStr(id).c_str(); }
	const std::string& IDNameStr(const ID* id) const;

	std::string ParamDecl(const FuncTypePtr& ft, const ProfileFunc* pf);
	const ID* FindParam(int i, const ProfileFunc* pf);

	bool IsNativeType(const TypePtr& t) const;
	const char* FullTypeName(const TypePtr& t);
	const char* TypeName(const TypePtr& t);
	const char* TypeType(const TypePtr& t);
	int TypeIndex(const TypePtr& t);
	void RecordAttributes(const AttributesPtr& attrs);

	const char* NativeAccessor(const TypePtr& t);
	const char* IntrusiveVal(const TypePtr& t);

	void AddInit(const IntrusivePtr<Obj>& o,
			const std::string& lhs, const std::string& rhs)
		{ AddInit(o.get(), lhs + " = " + rhs + ";"); }
	void AddInit(const Obj* o,
			const std::string& lhs, const std::string& rhs)
		{ AddInit(o, lhs + " = " + rhs + ";"); }
	void AddInit(const IntrusivePtr<Obj>& o, const std::string& init)
		{ AddInit(o.get(), init); }
	void AddInit(const Obj* o, const std::string& init);

	// For objects w/o initializations, but with dependencies.
	void AddInit(const IntrusivePtr<Obj>& o)	{ AddInit(o.get()); }
	void AddInit(const Obj* o);

	// Records the fact that the initialization of object o1 depends
	// on that of object o2.
	void NoteInitDependency(const IntrusivePtr<Obj>& o1,
				const IntrusivePtr<Obj>& o2)
		{ NoteInitDependency(o1.get(), o2.get()); }
	void NoteInitDependency(const Obj* o1, const IntrusivePtr<Obj>& o2)
		{ NoteInitDependency(o1, o2.get()); }
	void NoteInitDependency(const Obj* o1, const Obj* o2);
	void NoteNonRecordInitDependency(const IntrusivePtr<Obj>& o,
						const TypePtr& t)
		{
		if ( t && t->Tag() != TYPE_RECORD )
			NoteInitDependency(o, t);
		}

	void StartBlock();
	void EndBlock(bool needs_semi = false);

	void Emit(const std::string& str) const
		{
		Indent();
		fprintf(write_file, "%s", str.c_str());
		NL();
		}

	void Emit(const std::string& fmt, const std::string& arg) const
		{
		Indent();
		fprintf(write_file, fmt.c_str(), arg.c_str());
		NL();
		}

	void Emit(const std::string& fmt, const std::string& arg1,
			const std::string& arg2) const
		{
		Indent();
		fprintf(write_file, fmt.c_str(), arg1.c_str(), arg2.c_str());
		NL();
		}

	void Emit(const std::string& fmt, const std::string& arg1,
			const std::string& arg2, const std::string& arg3) const
		{
		Indent();
		fprintf(write_file, fmt.c_str(), arg1.c_str(), arg2.c_str(),
			arg3.c_str());
		NL();
		}

	void Emit(const std::string& fmt, const std::string& arg1,
			const std::string& arg2, const std::string& arg3,
			const std::string& arg4) const
		{
		Indent();
		fprintf(write_file, fmt.c_str(), arg1.c_str(), arg2.c_str(),
			arg3.c_str(), arg4.c_str());
		NL();
		}

	std::string GlobalName(const std::string& g, const char* suffix)
		{
		return Canonicalize(g.c_str()) + "__" + suffix;
		}

	std::string LocalName(const ID* l) const;

	std::string Canonicalize(const char* name) const;

	std::string Fmt(int i)
		{
		char d_s[64];
		snprintf(d_s, sizeof d_s, "%d", i);
		return std::string(d_s);
		}

	std::string Fmt(hash_type u)
		{
		char d_s[64];
		snprintf(d_s, sizeof d_s, "%lluULL", u);
		return std::string(d_s);
		}

	std::string Fmt(double d)
		{
		char d_s[64];
		snprintf(d_s, sizeof d_s, "%lf", d);
		return std::string(d_s);
		}

	std::string CPPEscape(const std::string& s) const
		{ return CPPEscape(s.c_str()); }
	std::string CPPEscape(const char* s) const;

	void NL() const
		{
		fputc('\n', write_file);
		}

	void Indent() const;

	std::vector<FuncInfo>& funcs;
	FILE* write_file;

	// Maps global names (not identifiers) to the names we use for them.
	std::unordered_map<std::string, std::string> globals;

	// Which event handlers we've declared.
	std::unordered_set<std::string> declared_events;

	// Maps event names to the names we use for them.
	std::unordered_map<std::string, std::string> events;

	// Globals that correspond to variables, not functions.
	std::unordered_set<const ID*> global_vars;

	// Globals that correspond to functions that are also used as
	// variables (i.e., in contexts where they're not just a function
	// being directly called.
	std::unordered_set<const ID*> global_func_vars;

	// Functions that we've declared/compiled.
	std::unordered_set<std::string> compiled_funcs;

	// Maps function names to hashes of bodies.
	std::unordered_map<std::string, unsigned long long> body_hashes;

	// Script functions that we are able to compile.  We compute
	// these ahead of time so that when compiling script function A
	// which makes a call to script function B, we know whether
	// B will indeed be compiled, or if it'll be interpreted due to
	// it including some functionality we don't currently support
	// for compilation.
	//
	// Indexed by the name of the function.
	std::unordered_set<std::string> compilable_funcs;

	// BiF's that we've processed.
	std::unordered_set<std::string> bifs;

	// Same for locals, for the function currently being compiled.
	std::unordered_map<const ID*, std::string> locals;

	// The function's parameters.  Tracked so we don't re-declare them.
	std::unordered_set<const ID*> params;

	// Maps (non-native) constants to associated C++ globals.
	std::unordered_map<const ConstExpr*, std::string> const_exprs;

	// Maps string representations of (non-native) constants to
	// associated C++ globals.
	std::unordered_map<std::string, std::string> constants;

	// Maps an object requiring initialization to its initializers.
	std::unordered_map<const Obj*, std::vector<std::string>> obj_inits;

	// Maps an object requiring initializations to its dependencies
	// on other such objects.
	std::unordered_map<const Obj*, std::unordered_set<const Obj*>> obj_deps;

	// A list of pre-initializations (those potentially required by
	// other initializations, and that themselves have no dependencies).
	std::vector<std::string> pre_inits;

	// Maps types to indices in the global "types__CPP" array.
	CPPTracker<const Type*, TypePtr> types = "types";

	// Used to prevent analysis of mutually-referring types from
	// leading to infinite recursion.
	std::unordered_set<const Type*> processed_types;

	// Similar for attributes, so we can reconstruct record types.
	CPPTracker<const Attributes*, AttributesPtr> attributes = "attrs";

	// Expressions for which we need to generate initialization-time
	// code.  Currently, these are only expressions appearing in
	// attributes.
	CPPTracker<const Expr*, ExprPtr> init_exprs = "gen_init_expr";

	// Maps function bodies to the names we use for them.
	std::unordered_map<const Stmt*, std::string> body_names;

	// If non-zero, provides a tag used for auxiliary/additional
	// compilation units.
	int addl_tag = 0;

	// Return type of the function we're currently compiling.
	TypePtr ret_type = nullptr;

	// Whether we're parsing a hook.
	bool in_hook = false;

	// Nested level of loops/switches for which "break"'s should be
	// C++ breaks rather than a "hook" break.
	int break_level = 0;

	int block_level = 0;
};

} // zeek::detail
