#include <lua.hpp>
//#include "lua/map.h"
//#include "lua/region.h"
//#include "lua/nbt.h"
#include "lua/lua_helper.h"

void register_classes(lua_State *state)
{
	//register_map(state);
	//register_region(state);
	//register_nbt(state);
}

void register_global_constants(lua_State *state, const ConstantValue *c, int count)
{
	for(int i = 0; i < count; i++)
	{
		if(c[i].getType() == TYPE_INT)
			lua_pushnumber(state, c[i].getValue<int>());
		else if(c[i].getType() == TYPE_NUM)
			lua_pushnumber(state, c[i].getValue<double>());
		else if(c[i].getType() == TYPE_STR)
			lua_pushstring(state, c[i].getValue<const char*>());
		else
			lua_pushnil(state);
		
		lua_setglobal(state, c[i].getName());
	}
}

void register_class_constants_(lua_State *state, const std::string &class_name, ConstantValue *c, int count)
{
	lua_getglobal(state, class_name.c_str());
	if(lua_isnil(state, -1))
	{
		luaL_error(state, "attempt to register class constants in class (%s) that does not exist", class_name.c_str());
		return;
	}
	
	for(int i = 0; i < count; i++)
	{
		lua_pushstring(state, c[i].getName());
		
		if(c[i].getType() == TYPE_INT)
			lua_pushnumber(state, c[i].getValue<int>());
		else if(c[i].getType() == TYPE_NUM)
			lua_pushnumber(state, c[i].getValue<double>());
		else if(c[i].getType() == TYPE_STR)
			lua_pushstring(state, c[i].getValue<const char*>());
		else
			lua_pushnil(state);
		
		lua_rawset(state, -3);
	}
	
	lua_pop(state, 1);
}

void lua_stack_dump(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t) {

			case LUA_TSTRING:  /* strings */
			printf("`%s'", lua_tostring(L, i));
			break;

			case LUA_TBOOLEAN:  /* booleans */
			printf(lua_toboolean(L, i) ? "true" : "false");
			break;

			case LUA_TNUMBER:  /* numbers */
			printf("%g", lua_tonumber(L, i));
			break;

			default:  /* other values */
			printf("%s", lua_typename(L, t));
			break;

		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */
}
