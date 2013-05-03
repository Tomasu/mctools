#include <lua.hpp>
#include <cstring>
#include "NBT_Debug.h"

#define _strify(str) #str
#define _concat(str1, str2) str1 ## str2

#define register_class(state, class_name, meta_table, class_table) do { \
	luaL_newlib(state, class_table); /* 1 */ \
	luaL_newmetatable(state, #class_name); /* 21 */ \
	luaL_setfuncs(state, meta_table, 0); /* 21 */ \
	lua_pushliteral(state, "__index"); /* 321 */ \
	lua_pushvalue(state, -3); /* 4321 */ \
	lua_rawset(state, -3); /* 21 */ \
	lua_pushliteral(state, "__metatable"); /* 321 */ \
	lua_pushvalue(state, -3); /* 4321 */ \
	lua_rawset(state, -3); /* 21 */ \
	lua_pop(state, 1); \
	lua_setglobal(state, #class_name); \
} while(0)

void register_classes(lua_State *state);

#define create_lua_object(state, class_name, obj) do { \
	class_name **udata__ = (class_name **)lua_newuserdata(state, sizeof(_concat(Lua,class_name))); \
	new (*udata__) _concat(Lua,class_name)(state, obj); \
	*udata__ = obj; \
	\
	luaL_getmetatable(state, #class_name); \
	lua_setmetatable(state, -2); \
	/*lua_stack_dump(state);*/ \
} while(0)

#define get_lua_object(state, class_name) *(_concat(Lua,class_name) **)luaL_checkudata(state, 0, #class_name)

#define destroy_lua_object(state, class_name) do { \
	_concat(Lua, class_name) *udata__ = *(_concat(Lua, class_name) **)luaL_checkudata(state, 0, #class_name); \
	udata__->~_concat(Lua, class_name)(); \
} while(0)

enum CONST_TYPE {
	TYPE_UNK = 0,
	TYPE_INT,
	TYPE_NUM,
	TYPE_STR
};

class ConstantValue {
	public:
		ConstantValue(const ConstantValue &copy) : name(copy.name), type(copy.type) { v = copy.v; }
		ConstantValue(const char *name, int i) : name(name), type(TYPE_INT) { v.intValue = i; }
		ConstantValue(const char *name, double n) : name(name), type(TYPE_NUM) { v.numValue = n; }
		ConstantValue(const char *name, const char *s) : name(name), type(TYPE_STR) { v.strValue = strdup(s); }
	
		const char *getName() const { return name; }
		
		template<typename T>
		T getValue() const { return *(T*)(void*)&v; }
		
		CONST_TYPE getType() const { return type; }
		
	private:
		ConstantValue() : name("UNK"), type(TYPE_UNK) { }
		const char *name;
		CONST_TYPE type;
		union {
			int intValue;
			double numValue;
			const char *strValue;
		} v;
};

#define DeclareConstant(c) ConstantValue(#c, c)

void register_constants(lua_State *state, const ConstantValue *c, int count);

void lua_stack_dump(lua_State *L);
