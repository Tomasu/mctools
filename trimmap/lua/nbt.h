#ifndef LUANBT_H_GUARD
#define LUANBT_H_GUARD

#include <lua.hpp>

#include "NBT_Tag.h"
#include "NBT_Tag_Compound.h"

class LuaNBT_Tag
{
	public:
		LuaNBT_Tag(lua_State *, NBT_Tag *nbt) : nbt(nbt) { }
		int type() { return nbt->type(); }
		bool named() { return nbt->named(); }
		const std::string name() { return nbt->name(); }
		NBT_Tag *parent() { return nbt->parent(); }
		int row() { return nbt->row(); }
		bool hasKey(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->hasKey(key); }
		NBT_Tag *get(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->get(key); }
		int8_t getByte(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getByte(key); }
		int16_t getShort(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getShort(key); }
		int32_t getInt(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getInt(key); }
		int64_t getLong(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getLong(key); }
		float getFloat(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getFloat(key); }
		double getDouble(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getDouble(key); }
		std::string getString(const std::string &key) { return ((NBT_Tag_Compound*)nbt)->getString(key); }
		
		NBT_Tag *getNBT() { return nbt; }
		
	private:
		NBT_Tag *nbt;
};

void register_nbt(lua_State *state);

#endif /* LUANBT_H_GUARD */
