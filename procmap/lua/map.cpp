#include "Map.h"
#include "Region.h"
#include "lua/lua_helper.h"
#include "lua/region.h"
#include "lua/map.h"
#include "lua/region.h"

#define MAP_CLASSNAME Map

static int Map_firstRegion(lua_State *state)
{
	LuaMap *map = get_lua_object(state, MAP_CLASSNAME);
	MCRegion *region = map->firstRegion();
	create_lua_object(state, Region, region);
	return 1;
}

static int Map_nextRegion(lua_State *state)
{
	LuaMap *map = get_lua_object(state, MAP_CLASSNAME);
	MCRegion *region = map->nextRegion();
	create_lua_object(state, Region, region);
	return 1;
}

static int Map_numRegions(lua_State *state)
{
	LuaMap *map = get_lua_object(state, MAP_CLASSNAME);
	lua_pushinteger(state, map->numRegions());
	return 1;
}

static int Map_destroy(lua_State *state)
{
	destroy_lua_object(state, MAP_CLASSNAME);
	return 0;
}

static const luaL_Reg map_meta_methods[] = {
	{"__gc", &Map_destroy },
	{ 0, 0 }
};

static const luaL_Reg map_methods[] = {
	{ "firstRegion", &Map_firstRegion },
	{ "nextRegion", &Map_nextRegion },
	{ "numRegions", &Map_numRegions },
	{ 0, 0 }
};

void register_map(lua_State *state)
{
	register_class(state, MAP_CLASSNAME, map_meta_methods, map_methods);
}
