#include <unistd.h>

#include <cstring>
#include <cerrno>

#include <cmath>
#include <climits>

#include "Level.h"
#include "Map.h"
#include "Player.h"
#include "Block.h"
#include "MCRegion.h"
#include "NBT_Debug.h"
#include "BitMap.h"
#include "widen.h"
#include "clean.h"
#include "process.h"

uint64_t block_counts[BLOCK_COUNT];

static const std::vector<uint32_t> overworld_keep_block_ids = {
	BLOCK_GLASS, BLOCK_LAPIS_BLOCK, BLOCK_JUKEBOX, BLOCK_BED, BLOCK_GOLDEN_RAIL,
	BLOCK_DETECTOR_RAIL, BLOCK_STICKY_PISTON, BLOCK_PISTON, BLOCK_PISTON_EXTENSION,
	BLOCK_WOOL, BLOCK_GOLD_BLOCK, BLOCK_IRON_BLOCK, BLOCK_DIAMOND_BLOCK, BLOCK_LIT_FURNACE,
	BLOCK_STANDING_SIGN, BLOCK_STONE_PRESSURE_PLATE, BLOCK_WOODEN_PRESSURE_PLATE,
	BLOCK_UNLIT_REDSTONE_TORCH, BLOCK_REDSTONE_TORCH, BLOCK_PORTAL, BLOCK_UNPOWERED_REPEATER,
	BLOCK_POWERED_REPEATER, BLOCK_ENDER_CHEST, BLOCK_FENCE_GATE,
	BLOCK_ENCHANTING_TABLE, BLOCK_REDSTONE_LAMP, BLOCK_LIT_REDSTONE_LAMP,
	BLOCK_ENDER_CHEST, BLOCK_EMERALD_BLOCK,
	BLOCK_COMMAND_BLOCK, BLOCK_LIGHT_WEIGHTED_PRESSURE_PLATE, BLOCK_HEAVY_WEIGHTED_PRESSURE_PLATE,
	BLOCK_UNPOWERED_COMPARATOR, BLOCK_POWERED_COMPARATOR, BLOCK_DAYLIGHT_DETECTOR,
	BLOCK_REDSTONE_BLOCK, BLOCK_HOPPER, BLOCK_QUARTZ_BLOCK, BLOCK_QUARTZ_STAIRS,
	BLOCK_ACTIVATOR_RAIL
};

static const std::vector<uint32_t> nether_nkeep_block_ids = {
	BLOCK_BEDROCK, BLOCK_AIR, BLOCK_LAVA, BLOCK_FLOWING_LAVA, BLOCK_MOB_SPAWNER, BLOCK_GRAVEL,
	BLOCK_BROWN_MUSHROOM, BLOCK_RED_MUSHROOM, BLOCK_FIRE, BLOCK_MOB_SPAWNER, BLOCK_CHEST,
	BLOCK_NETHERRACK, BLOCK_SOUL_SAND, BLOCK_GLOWSTONE, BLOCK_NETHER_BRICK, BLOCK_NETHER_BRICK_FENCE,
	BLOCK_NETHER_BRICK_STAIRS, BLOCK_NETHER_WART, BLOCK_QUARTZ_ORE, BLOCK_TORCH
};

bool has_block(const std::vector<uint32_t> &blocks, uint32_t block_id)
{
	int n = blocks.size();

	for(int32_t i = 0; i < n ; i++)
	{
		if(blocks[i] == block_id)
			return true;
	}
	
	return false;
}

BitMap *build_bitmap(Map *map)
{
	int64_t min_x = LONG_MAX, max_x = -LONG_MAX;
	int64_t min_z = LONG_MAX, max_z = -LONG_MAX;
	uint64_t total_chunks = 0;
	
	MCRegion *region = map->firstRegion();
	while(region != 0)
	{
		//region->load();
		
		if(region->x() < min_x)
			min_x = region->x();
	
		if(region->z() < min_z)
			min_z = region->z();
	
		if(region->x() > max_x)
			max_x = region->x();
	
		if(region->z() > max_z)
			max_z = region->z();
	
		total_chunks += region->chunkCount();
		
		//region->unload();
		
		region = map->nextRegion();
	}
	
	BitMap *chunkBitMap = new BitMap((min_z-1)*32, (min_x-1)*32, (max_z+1)*32, (max_x+1)*32);
	
	uint64_t area = chunkBitMap->width() * chunkBitMap->height();
	
	NBT_Debug("found map of %lu chunks, area %lux%lu (%lu) chunks", total_chunks, chunkBitMap->width(), chunkBitMap->height(), area);
	
	return chunkBitMap;
}

int main(int argc, char **argv)
{
	NBT_Debug("begin");
	
	/*lua_State *state = init_lua();
	if(argc < 3)
	{
		printf("usage: %s <mappath> <scriptpath>\n", argv[0]);
		return 0;
	}*/
	
	Level *level = new Level();
	if(!level->load(argv[1]))
	{
		printf("failed to load level\n");
		return 0;
	}
	
	for(auto &map: level->maps())
	{
		printf("map: %s:%i\n", map->mapName().c_str(), map->dimension());
		//continue;
		
		// DO NOT process The End
		if(map->dimension() == 1)
			continue;
		
		BitMap *bitMap = build_bitmap(map);
		
		NBT_Debug("process map: %s", map->mapName().c_str());
		
		
		bool ret = false;
		
		if(map->dimension() == -1)
			ret = process_map(map, bitMap, nether_nkeep_block_ids, false);
		else
			ret = process_map(map, bitMap, overworld_keep_block_ids, true);
		
		if(!ret)
		{
			printf("failed to process map\n");
			return 0;
		}
		
		// save map spawn, but only if this map isn't the nether or the end
		if(map->dimension() != -1 && map->dimension() != 1)
			bitMap->set(map->spawnX(), map->spawnZ());
		
		// save players spawn and position
		std::vector<Player *> players = level->dimensionPlayers(map->dimension());
		for(auto &player: players)
		{
			bitMap->set(player->spawnX(), player->spawnZ());
			bitMap->set(player->xPos(), player->zPos());
		}
		
		// expand kept areas by a chunk around every kept chunk
		int radius = map->dimension() == -1 ? 4 : 10;
		BitMap *widened = widen_bitmap(bitMap, radius);
		delete bitMap;
		
		// delete unused chunks and regions
		clean_map(map, widened);
		
		printf("\n\n\nchunk block count:\n");
		for(int i = 0; i < BLOCK_COUNT; i++)
		{
			printf("%s: %lu\n", BlockName(i), block_counts[i]);
		}
		//break;
	}
	
	NBT_Debug("end");
	return 0;
}
