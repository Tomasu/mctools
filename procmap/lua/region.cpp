#include <lua.hpp>
#include "NBT_Debug.h"
#include "Region.h"
#include "Chunk.h"
#include "lua/lua_helper.h"
#include "lua/region.h"
#include "lua/chunk.h"

#define REGION_CLASSNAME Region

static int Region_destroy(lua_State *state)
{
	(void)state;
	NBT_Debug("");
	return 0;
}

static int Region_load(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	region->load();
	return 0;
}

static int Region_unload(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	region->unload();
	return 0;
}

static int Region_x(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	lua_pushinteger(state, region->x());
	return 1;
}

static int Region_z(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	lua_pushinteger(state, region->z());
	return 1;
}

static int Region_exists(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	lua_pushboolean(state, region->exists());
	return 1;
}

struct chunks_data
{
	LuaRegion *region;
	uint32_t idx;
};

static int Region_chunks_closure(lua_State *state)
{
	chunks_data *data = (chunks_data *)lua_touserdata(state, lua_upvalueindex(1));
	if(data->idx < data->region->chunks().size())
	{
		create_lua_object(state, Chunk, data->region->chunks().at(data->idx));
		data->idx++;
	}
	else
		lua_pushnil(state);
	
	return 1;
}

static int Region_chunks(lua_State *state)
{
	LuaRegion *region = get_lua_object(state, REGION_CLASSNAME);
	
	chunks_data *data = new chunks_data;
	data->region = region;
	data->idx = 0;
	
	lua_pushlightuserdata(state, data);
	lua_pushcclosure(state, &Region_chunks_closure, 1);
	
	return 1;
}

static const luaL_Reg region_meta_methods[] = {
	{"__gc", &Region_destroy },
	{ 0, 0 }
};

static const luaL_Reg region_methods[] = {
	{ "load", &Region_load },
	{ "unload", &Region_unload },
	{ "x", &Region_x },
	{ "z", &Region_z },
	{ "exists", &Region_exists },
	{ "chunks", &Region_chunks },
	{ 0, 0 }
};

void register_region(lua_State *state)
{
	register_class(state, REGION_CLASSNAME, region_meta_methods, region_methods);
}


#if 0

void register_foo_class(lua_State* L) {
    int lib_id, meta_id;

    /* newclass = {} */
    lua_createtable(L, 0, 0);
    lib_id = lua_gettop(L);

    /* metatable = {} */
    luaL_newmetatable(L, "Foo");
    meta_id = lua_gettop(L);
    luaL_setfuncs(L, _meta, 0);

    /* metatable.__index = _methods */
    luaL_newlib(L, _methods);
    lua_setfield(L, meta_id, "__index");    

    /* metatable.__metatable = _meta */
    luaL_newlib(L, _meta);
    lua_setfield(L, meta_id, "__metatable");

    /* class.__metatable = metatable */
    lua_setmetatable(L, lib_id);

    /* _G["Foo"] = newclass */
    lua_setglobal(L, "Foo");
}

#endif

