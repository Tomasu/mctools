#ifndef PROCESS_LUA_H_GUARD
#define PROCESS_LUA_H_GUARD

#include <lua.hpp>
#include "Map.h"

lua_State *init_lua();
bool process_map_lua(lua_State *state, Map *map, const char *scriptpath);

#endif /* PROCESS_LUA_H_GUARD */
