#ifndef LUAREGION_H_GUARD
#define LUAREGION_H_GUARD

#include <lua.hpp>
#include "Region.h"

void register_region(lua_State *state);

class Chunk;
class LuaRegion
{
	public:
		LuaRegion(lua_State *, Region *region) : region(region) { }
		
		bool load() { return region->load(); }
		void unload() { region->unload(); }
		bool save() { return region->save(); }
		bool save(const std::string &file) { return region->save(file); }
		
		const std::string &filePath() { return region->filePath(); }
		int x() { return region->x(); }
		int z() { return region->z(); }
		
		bool exists() { return region->exists(); }
		bool isOldFormat() { return region->isOldFormat(); }
		
		const std::vector <Chunk *> &chunks() { return region->chunks(); }
		
		void deleteChunk(Chunk *c) { region->deleteChunk(c); }
		
	private:
		Region *region;
};

#endif /* LUAREGION_H_GUARD */
