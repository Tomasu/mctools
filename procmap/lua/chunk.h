#ifndef LUA_CHUNK_H_GUARD
#define LUA_CHUNK_H_GUARD

#include <lua.hpp>

class NBT_Tag;

class LuaChunk
{
	public:
		LuaChunk(lua_State *, Chunk *chunk) : chunk(chunk) { }
		int getIdx() { return chunk->getIdx(); }
		uint32_t len() { return chunk->len(); }
		NBT_Tag *nbt() { return (NBT_Tag*)chunk->nbt(); }
		uint32_t offset() { return chunk->offset(); }
		int getTimestamp() { return chunk->getTimestamp(); }
		
		int x() { return chunk->x(); }
		int z() { return chunk->z(); }
		
	private:
		Chunk *chunk;
};

void register_chunk(lua_State *state);

#endif /* LUA_CHUNK_H_GUARD */
