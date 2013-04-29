#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <lua.hpp>

#include "Map.h"
#include "NBT_Tag.h"
#include "Region.h"
#include "Chunk.h"
#include "lua/map.h"
#include "lua/region.h"
#include "lua/nbt.h"
#include "lua/chunk.h"
#include "lua/lua_helper.h"
#include "NBT_Debug.h"

lua_State *init_lua()
{
	NBT_Debug("begin");
	lua_State *state = luaL_newstate();
	luaL_openlibs(state);
	
	//register_map(state);
	//register_region(state);
	register_chunk(state);
	register_nbt(state);
	
	NBT_Debug("end");
	return state;
}

bool load_script(lua_State *state, const char *name, const char *str)
{
	NBT_Debug("load script %s: %s", name, str);
	
	if(luaL_loadfile(state, str) != LUA_OK) // 1
	{
		const char *errstr = luaL_checklstring(state, 1, 0);
		printf("failed to load %s: %s\n", name, errstr);
		return false;
	}
	
	NBT_Debug("new table");
	lua_newtable(state); // script env  // 21
	lua_setglobal(state, name); // set global name for new table // 1

	lua_getglobal(state, name); // 21
	lua_newtable(state); // metatable // 321
	lua_getglobal(state, "_G"); // 4321
	lua_setfield(state, -2, "__index"); // set __index in metatable to _G // 321
	lua_setmetatable(state, -2); // set metatable for script env // 21

	NBT_Debug("set upvalue");
	lua_setupvalue(state, 1, 1); // set env for state // 21
	
	printf("run script %s\n", name);
	if(lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK) // run script // 1
	{
		const char *errstr = luaL_checklstring(state, 1, 0);
		printf("failed to run script %s: %s\n", name, errstr);
		return false;
	}

	NBT_Debug("end");
	return true;
}

bool run_fun(lua_State *state, const char *name, const char *fun, int nargs)
{	
	//NBT_Debug("begin");
	int stack_top = lua_gettop(state);
	
	lua_getglobal(state, name);
	lua_getfield(state, -1, fun);
	if(lua_isnil(state, -1))
	{
		printf("script %s function %s does not exist.\n", name, fun);
		//NBT_Debug("end");
		return true; // ignore function not existing, assume 
	}
	
	// remove global "name" table
	lua_remove(state, -2);
	
	for(int i = stack_top-nargs+1; i <= stack_top; i++)
	{
		//NBT_Debug("arg");
		lua_pushvalue(state, i);
	}
	
	//NBT_Debug("dump:\n");
	//lua_stack_dump(state);
	//NBT_Debug("\ncall func %s:%i", fun, nargs);
	if(lua_pcall(state, nargs, 1, 0) != LUA_OK)
	{
		const char *errstr = luaL_checklstring(state, -1, 0);
		//NBT_Debug("failed to call %s.foo: %s", fun,errstr);
		//NBT_Debug("end");
		luaL_error(state, "failed to call %s.%s: %s\n", name, fun, errstr);
		return false; // luaL_error doesn't return, return here to keep the compiler from complaining
	}
	
	//NBT_Debug("end");
	bool ret = lua_toboolean(state, -1);
	return ret;
}

std::vector<std::string> scripts;
bool run_funcs(lua_State *state, const char *fun, int nargs, bool exclude)
{
	//NBT_Debug("begin");
	// if exclude==true, short circuit on a false from any script
	//  else short circuit on a true from any script
	for(auto &script: scripts)
	{
		bool ret = run_fun(state, script.c_str(), fun, nargs);
		if(exclude)
		{
			if(!ret)
			{
				//NBT_Debug("end");
				return false;
			}
		}
		else
		{
			if(ret)
			{
				//NBT_Debug("end");
				return true;
			}
		}
	}

	//NBT_Debug("end");
	return exclude ? true : false;
}

bool load_scripts(lua_State *state, const char *scriptpath)
{
	NBT_Debug("begin");

	DIR *dh = opendir(scriptpath);
	if(!dh)
	{
		printf("failed to list scriptpath: %s\n", strerror(errno));
		return false;
	}
	
	int path_len = strlen(scriptpath);
	struct dirent *ent = 0;
	
	while((ent = readdir(dh)))
	{
		// skip hidden files
		if(ent->d_name[0] == '.')
			continue;
		
		// skip backup files
		int name_len = strlen(ent->d_name);
		if(ent->d_name[name_len-1] == '~')
			continue;
		
		// if we have an extension, delete it.
		char *bname = strdup(ent->d_name);
		char *ptr = strrchr(bname, '.');
		if(ptr) *ptr = 0;
		
		char *fn = (char *)calloc(1, path_len + name_len + 2);
		sprintf(fn, "%s/%s", scriptpath, ent->d_name);
		
		struct stat s;
		if(stat(fn, &s) != 0)
		{
			printf("failed to stat %s: %s\n", ent->d_name, strerror(errno));
			return false;
		}
		
		if(!S_ISREG(s.st_mode))
		{
			printf("ent %s is not a regular file\n", ent->d_name);
			continue;
		}
		
		if(!load_script(state, bname, fn))
		{
			printf("failed to load script %s\n", ent->d_name);
			return false;
		}
		
		scripts.push_back(bname);
	}
	
	NBT_Debug("end");
	return true;
}

bool process_region(lua_State *state, Region *region)
{
	//NBT_Debug("begin");
	if(!region->load())
	{
		printf("failed to load region\n");
		return false;
	}
	
	for(auto &chunk: region->chunks())
	{
		//NBT_Debug("chunk");
		int top = lua_gettop(state);
		create_lua_object(state, Chunk, chunk);
		//NBT_Debug("pre top: %i, post top: %i", top, lua_gettop(state));
		//lua_pushvalue(state, -1);
		
		//bool include = run_funcs(state, "include", 1, true);
		//if(!include)
		//	// delete chunk
		
		//lua_stack_dump(state);
		//NBT_Debug("run_funcs:");
		bool keep = run_funcs(state, "include", 1, false);
		if(!keep)
			region->deleteChunk(chunk);
		
		lua_pop(state, 2);
	}
	region->unload();
	
	//NBT_Debug("end");
	return true;
}

bool process_map(lua_State *state, Map *map, const char *scriptpath)
{
	//NBT_Debug("begin");
	if(!load_scripts(state, scriptpath))
		return false;
	
	Region *region = map->firstRegion();
	while(region != 0)
	{
		if(!process_region(state, region))
		{
			printf("failed to process region\n");
			return false;
		}
		region = map->nextRegion();
	}
	
	//NBT_Debug("end");
	return true;
}

int main(int argc, char **argv)
{
	NBT_Debug("begin");
	
	lua_State *state = init_lua();
	if(argc < 3)
	{
		printf("usage: %s <mappath> <scriptpath>\n", argv[0]);
		return 0;
	}
	
	Map *map = new Map(argv[1]);
	if(!map->load())
	{
		printf("failed to load map\n");
		return 0;
	}
	
	NBT_Debug("process map");
	if(!process_map(state, map, argv[2]))
	{
		printf("failed to process map\n");
		return 0;
	}
	
	//if(!map->save())
	//{
	//	printf("failed to save map\n");
	//	return 0;
	//}

	NBT_Debug("end");
	return 0;
}
