#include "lua/lua_helper.h"
#include "lua/nbt.h"
#include "NBT_Tag.h"
#include "NBT_Tag_Compound.h"
#include "NBT_Tag_Byte.h"
#include "NBT_Tag_Short.h"
#include "NBT_Tag_Int.h"
#include "NBT_Tag_Long.h"
#include "NBT_Tag_Double.h"
#include "NBT_Tag_Float.h"
#include "NBT_Tag_String.h"

#define NBTTAG_CLASSNAME NBT_Tag

const ConstantValue constants[] = {
	DeclareConstant(TAG_UNKNOWN),
	DeclareConstant(TAG_End),
	DeclareConstant(TAG_Byte),
	DeclareConstant(TAG_Short),
	DeclareConstant(TAG_Int),
	DeclareConstant(TAG_Long),
	DeclareConstant(TAG_Float),
	DeclareConstant(TAG_Double),
	DeclareConstant(TAG_Byte_Array),
	DeclareConstant(TAG_String),
	DeclareConstant(TAG_List),
	DeclareConstant(TAG_Compound),
	DeclareConstant(TAG_Int_Array),
	DeclareConstant(TAG_LAST_ITEM)
};

static int NBT_destroy(lua_State *state)
{
	destroy_lua_object(state, NBTTAG_CLASSNAME);
	return 0;
}

static int NBT_type(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	lua_pushinteger(state, tag->type());
	return 1;
}

static int NBT_named(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	lua_pushboolean(state, tag->named());
	return 1;
}

static int NBT_name(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	lua_pushstring(state, tag->name().c_str());
	return 1;
}

static int NBT_parent(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	if(tag->parent())
		create_lua_object(state, NBT_Tag, tag->parent());
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_row(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	lua_pushinteger(state, tag->row());
	return 1;
}

static int NBT_hasKey(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	luaL_argcheck(state, tag->type() == TAG_Compound, 1, "tag must be Compound type");
	const char *key = luaL_checklstring(state, 2, 0);
	lua_pushboolean(state, ((NBT_Tag_Compound *)tag)->hasKey(key));
	return 1;
}

static int NBT_get(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
		create_lua_object(state, NBT_Tag, child);
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getByte(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Byte, 2, "not a TAG_Byte");
		lua_pushinteger(state, ((NBT_Tag_Byte*)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getShort(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Short, 2, "not a TAG_Short");
		lua_pushinteger(state, ((NBT_Tag_Short *)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getInt(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Int, 2, "not a TAG_Int");
		lua_pushinteger(state, ((NBT_Tag_Int *)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getLong(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Long, 2, "not a TAG_Long");
		lua_pushinteger(state, ((NBT_Tag_Long *)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getFloat(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Float, 2, "not a TAG_Float");
		lua_pushnumber(state, ((NBT_Tag_Float *)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getDouble(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_Double, 2, "not a TAG_Double");
		lua_pushnumber(state, ((NBT_Tag_Double *)child)->value());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_getString(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	const char *key = luaL_checklstring(state, 2, 0);
	NBT_Tag *child = ((NBT_Tag_Compound *)tag)->get(key);
	if(child)
	{
		luaL_argcheck(state, child->type() == TAG_String, 2, "not a TAG_String");
		std::string str = ((NBT_Tag_String *)child)->value();
		lua_pushlstring(state, str.c_str(), str.length());
	}
	else
		lua_pushnil(state);
	
	return 1;
}

struct child_data {
	NBT_Tag_Compound *tag;
	int32_t idx;
};

static int NBT_children_closure(lua_State *state)
{
	child_data *data = (child_data *)lua_touserdata(state, lua_upvalueindex(1));
	if(data->idx < data->tag->count())
	{
		create_lua_object(state, NBT_Tag, data->tag->childAt(data->idx));
		data->idx++;
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int NBT_children(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);

	child_data *data = new child_data;
	data->tag = (NBT_Tag_Compound *)tag;
	data->idx = 0;
	
	lua_pushlightuserdata(state, data);
	lua_pushcclosure(state, &NBT_children_closure, 1);
	
	return 1;
}

static int NBT_childCount(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	luaL_argcheck(state, tag->type() == TAG_Compound, 1, "not a TAG_Compound");
	lua_pushinteger(state, ((NBT_Tag_Compound *)tag)->count());
	
	return 1;
}

static int NBT_childAt(lua_State *state)
{
	LuaNBT_Tag *tag = get_lua_object(state, NBTTAG_CLASSNAME);
	luaL_argcheck(state, tag->type() == TAG_Compound, 1, "not a TAG_Compound");
	int idx = luaL_checkinteger(state, 2);
	create_lua_object(state, NBT_Tag, ((NBT_Tag_Compound *)tag)->childAt( idx ));
	
	return 1;
}

static const luaL_Reg nbt_meta_methods[] = {
	{ "__gc", &NBT_destroy },
	{ 0, 0 }
};

static const luaL_Reg nbt_methods[] = {
	{ "type", &NBT_type },
	{ "named", &NBT_named },
	{ "name", &NBT_name },
	{ "parent", &NBT_parent },
	{ "row", &NBT_row },
	
	{ "hasKey", &NBT_hasKey },
	{ "get", &NBT_get },
	{ "getByte", &NBT_getByte },
	{ "getShort", &NBT_getShort },
	{ "getInt", &NBT_getInt },
	{ "getLong", &NBT_getLong },
	{ "getFloat", &NBT_getFloat },
	{ "getDouble", &NBT_getDouble },
	{ "getString", &NBT_getString },
	
	{ "children", &NBT_children },
	{ "childCount", &NBT_childCount },
	{ "childAt", &NBT_childAt },

	{ 0, 0 },
};

void register_nbt(lua_State *state)
{
	register_constants(state, constants, sizeof(constants) / sizeof(ConstantValue));
	register_class(state, NBTTAG_CLASSNAME, nbt_meta_methods, nbt_methods);
}
