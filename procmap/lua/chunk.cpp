#include "Chunk.h"
#include "chunk.h"
#include "NBT_Debug.h"
#include "NBT_Tag.h"
#include "lua/lua_helper.h"
#include "lua/nbt.h"

#define CHUNK_CLASSNAME Chunk

static int Chunk_destroy(lua_State *state)
{
	destroy_lua_object(state, Chunk);
	return 0;
}

static int Chunk_idx(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->getIdx());
	return 1;
}

static int Chunk_len(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->len());
	return 1;
}

static int Chunk_nbt(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	create_lua_object(state, NBT_Tag, chunk->nbt());
	return 1;
}

static int Chunk_offset(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->offset());
	return 1;
}

static int Chunk_timestamp(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->getTimestamp());
	return 1;
}

static int Chunk_x(lua_State *state)
{
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->x());
	return 1;
}

static int Chunk_z(lua_State *state)
{
	//NBT_Debug("");
	LuaChunk *chunk = get_lua_object(state, CHUNK_CLASSNAME);
	lua_pushinteger(state, chunk->z());
	return 1;
}

static const luaL_Reg chunk_meta_methods[] = {
	{ "__gc", &Chunk_destroy },
	{ 0, 0 }
};

static const luaL_Reg chunk_methods[] = {
	{ "x", &Chunk_x },
	{ "z", &Chunk_z },
	{ "timestamp", &Chunk_timestamp },
	{ "nbt", &Chunk_nbt },
	{ "idx", &Chunk_idx },
	{ "offset", &Chunk_offset },
	{ "len", &Chunk_len },
	{ 0, 0 }
};

void register_chunk(lua_State *state)
{
	register_class(state, CHUNK_CLASSNAME, chunk_meta_methods, chunk_methods);
}
