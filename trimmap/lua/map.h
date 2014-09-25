#ifndef LUA_MAP_H_GUARD
#define LUA_MAP_H_GUARD

#include <lua.hpp>

class MCRegion;

#include "Map.h"

class LuaMap
{
	public:
		LuaMap(lua_State *, Map *map) : map(map) { }
		Region *firstRegion() { return map->firstRegion(); }
		Region *nextRegion() { return map->nextRegion(); }
		int numRegions() { return map->numRegions(); }
		
	private:
		Map *map;
};

void register_map(lua_State *state);

#endif /* LUA_MAP_H_GUARD */
