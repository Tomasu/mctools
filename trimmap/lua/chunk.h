#ifndef LUA_CHUNK_H_GUARD
#define LUA_CHUNK_H_GUARD

#include <LuaGlue/LuaGlue.h>

class Chunk;
class NBT_Tag_Byte_Array;

class LuaChunk
{
	public:
		LuaChunk(Chunk *chunk);
		~LuaChunk() { }
		
		int xPos() { return level->getInt("xPos"); }
		int zPos() { return level->getInt("zPos"); }
		
		int timestamp() { return chunk->getTimestamp(); }
		
		int block(int x, int z);
		
	private:
		Chunk *chunk;
		NBT_Tag_Compound *level;
}

void register_chunk(LuaGlue &glue);

#endif /* LUA_CHUNK_H_GUARD */
