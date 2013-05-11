#include <unistd.h>

#include <cstring>
#include <cerrno>

#include <cmath>
#include <climits>

#include "Map.h"
#include "Block.h"
#include "Region.h"
#include "NBT_Debug.h"
#include "BitMap.h"
#include "widen.h"
#include "clean.h"
#include "process.h"

uint64_t block_counts[BLOCK_COUNT];

const uint32_t keep_block_ids[] = {
	BLOCK_GLASS, BLOCK_LAPIS_BLOCK, BLOCK_MUSIC, BLOCK_BED, BLOCK_POWERED_RAIL,
	BLOCK_DETECTOR_RAIL, BLOCK_PISTON_STICKY_BASE, BLOCK_PISTON_BASE, BLOCK_PISTON_EXTENSION,
	BLOCK_WOOL, BLOCK_PISTON_MOVING, BLOCK_GOLD_BLOCK, BLOCK_IRON_BLOCK,
	BLOCK_DIAMOND_BLOCK, BLOCK_FURNACE_BURNING,
	BLOCK_SIGN_POST, BLOCK_PRESSURE_PLATE_STONE, BLOCK_PRESSURE_PLATE_PLANKS,
	BLOCK_REDSTONE_IDLE_TORCH, BLOCK_REDSTONE_ACTIVE_TORCH, BLOCK_PORTAL, BLOCK_REDSTONE_REPEATER_IDLE,
	BLOCK_REDSTONE_REPEATER_ACTIVE, BLOCK_LOCKED_CHEST, BLOCK_FENCE_GATE,
	BLOCK_ENCHANTMENT_TABLE, BLOCK_REDSTONE_LAMP_IDLE, BLOCK_REDSTONE_LAMP_ACTIVE,
	BLOCK_ENDER_CHEST, BLOCK_EMERALD_BLOCK,
	BLOCK_COMMAND_BLOCK, BLOCK_PRESSURE_PLATE_GOLD, BLOCK_PRESSURE_PLATE_IRON,
	BLOCK_REDSTONE_COMPARATOR_IDLE, BLOCK_REDSTONE_COMPARATOR_ACTIVE, BLOCK_DAYLIGHT_SENSOR,
	BLOCK_REDSTONE_BLOCK, BLOCK_HOPPER_BLOCK, BLOCK_NETHER_QUARTZ_BLOCK, BLOCK_NETHER_QUARTZ_STAIRS,
	BLOCK_ACTIVATOR_RAIL
};

static const uint32_t keep_block_ids_count = sizeof(keep_block_ids) / sizeof(uint32_t);

bool keep_block(uint32_t block_id)
{
	for(uint32_t i = 0; i < keep_block_ids_count; i++)
	{
		if(keep_block_ids[i] == block_id)
			return true;
	}
	
	return false;
}

BitMap *build_bitmap(Map *map)
{
	int64_t min_x = LONG_MAX, max_x = -LONG_MAX;
	int64_t min_z = LONG_MAX, max_z = -LONG_MAX;
	uint64_t total_chunks = 0;
	
	Region *region = map->firstRegion();
	while(region != 0)
	{
		if(region->x() < min_x)
			min_x = region->x();
	
		if(region->z() < min_z)
			min_z = region->z();
	
		if(region->x() > max_x)
			max_x = region->x();
	
		if(region->z() > max_z)
			max_z = region->z();
	
		total_chunks += region->chunks().size();
			
		region = map->nextRegion();
	}
	
	BitMap *chunkBitMap = new BitMap(min_z*32, min_x*32, max_z*32, max_x*32);
	
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
	
	Map *map = new Map(argv[1]);
	if(!map->load())
	{
		printf("failed to load map\n");
		return 0;
	}
	
	BitMap *bitMap = build_bitmap(map);
	
	NBT_Debug("process map");
	if(!process_map(map, bitMap))
	{
		printf("failed to process map\n");
		return 0;
	}
	
	// expand kept areas by a chunk around every kept chunk
	widen_bitmap(bitMap);
	
	// delete unused chunks and regions
	clean_map(map, bitMap);

	printf("\n\n\nchunk block count:\n");
	for(int i = 0; i < BLOCK_COUNT; i++)
	{
		printf("%s: %lu\n", BlockName(i), block_counts[i]);
	}
	
	NBT_Debug("end");
	return 0;
}
